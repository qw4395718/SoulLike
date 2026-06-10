"""
SoulLike 联机召唤系统 — 中间匹配服务
作用：连接多个 UE4 游戏实例，管理召唤标记的注册/查询/转发

启动：python match_server.py
默认监听 127.0.0.1:7777
"""

import socket
import json
import threading
import uuid
import sys

HOST = '127.0.0.1'
PORT = 7777

# ===== 服务器状态（纯内存） =====
instances = {}        # instance_id -> {"conn": socket, "map": str, "ip": str, "port": int}
signs = {}            # sign_id -> dict (完整的标记数据)
conn_owner = {}       # socket -> instance_id (反向查找)


def send_json(conn, data):
    """向一个连接发送JSON消息"""
    try:
        msg = json.dumps(data, ensure_ascii=False) + '\n'
        conn.sendall(msg.encode('utf-8'))
    except Exception as e:
        print(f"[错误] 发送失败: {e}")


def clean_instance(instance_id):
    """清理一个实例的所有数据"""
    if instance_id in instances:
        to_remove = [sid for sid, s in signs.items() if s.get('instance_id') == instance_id]
        for sid in to_remove:
            del signs[sid]
        conn = instances[instance_id]['conn']
        if conn in conn_owner:
            del conn_owner[conn]
        del instances[instance_id]
        print(f"[清理] 实例 {instance_id} 已断开，移除 {len(to_remove)} 个标记")


def handle_client(conn, addr):
    """处理一个客户端连接的消息循环"""
    print(f"[连接] 来自 {addr}")
    buffer = b""
    instance_id = None

    try:
        while True:
            data = conn.recv(4096)
            if not data:
                break

            buffer += data
            while b'\n' in buffer:
                line, buffer = buffer.split(b'\n', 1)
                line = line.strip()
                if not line:
                    continue

                try:
                    msg = json.loads(line.decode('utf-8'))
                    instance_id = process_message(conn, msg) or instance_id
                except json.JSONDecodeError as e:
                    print(f"[错误] JSON解析失败: {e} | 原始: {line[:200]}")

    except (ConnectionResetError, BrokenPipeError):
        pass
    finally:
        if instance_id:
            clean_instance(instance_id)
        conn.close()


def process_message(conn, msg):
    """处理一条JSON消息，返回可能的instance_id"""
    msg_type = msg.get('type', '')

    # === 1. 世界实例管理 ===
    if msg_type == 'register_instance':
        instance_id = msg['instance_id']
        instances[instance_id] = {
            'conn': conn,
            'map': msg.get('map', ''),
            'ip': msg.get('ip', ''),
            'port': msg.get('port', 0),
        }
        conn_owner[conn] = instance_id
        print(f"[实例] {instance_id} 注册: 地图={msg.get('map', '')} "
              f"地址={msg.get('ip', '')}:{msg.get('port', 0)}")
        send_json(conn, {'type': 'register_instance_ack', 'success': True})
        return instance_id

    elif msg_type == 'unregister_instance':
        instance_id = conn_owner.get(conn)
        if instance_id:
            clean_instance(instance_id)
            print(f"[实例] {instance_id} 主动注销")
        return None

    # === 2. 标记管理 ===
    elif msg_type == 'register_sign':
        instance_id = conn_owner.get(conn)
        if not instance_id:
            send_json(conn, {'type': 'register_sign_ack', 'success': False,
                             'error': '未注册实例'})
            return None

        sign_id = str(uuid.uuid4())
        sign_data = msg.get('sign_data', {})
        sign_data['sign_id'] = sign_id
        sign_data['instance_id'] = instance_id
        signs[sign_id] = sign_data
        print(f"[标记] {instance_id} 放置标记 {sign_id[:8]}... "
              f"当前总标记: {len(signs)}")
        send_json(conn, {'type': 'register_sign_ack', 'sign_id': sign_id,
                         'success': True})
        return None

    elif msg_type == 'unregister_sign':
        sign_id = msg.get('sign_id')
        if sign_id in signs:
            del signs[sign_id]
            print(f"[标记] 移除 {sign_id[:8]}... 剩余: {len(signs)}")
        return None

    elif msg_type == 'query_signs':
        instance_id = conn_owner.get(conn)
        map_name = msg.get('map', '')
        level = msg.get('level', 0)
        weapon_level = msg.get('weapon_level', 0)

        results = []
        for sid, s in signs.items():
            if map_name and s.get('map') != map_name:
                continue
            if instance_id and s.get('instance_id') == instance_id:
                continue
            if level and abs(int(s.get('owner_level', 0)) - int(level)) > 10:
                continue
            if weapon_level and abs(int(s.get('weapon_level', 0)) - int(weapon_level)) > 2:
                continue
            results.append(s)

        send_json(conn, {'type': 'query_signs_result', 'signs': results})
        print(f"[查询] {instance_id} 查询: 地图={map_name}, "
              f"结果={len(results)}个")
        return None

    # === 3. 召唤请求 ===
    elif msg_type == 'request_summon':
        sign_id = msg.get('sign_id')
        requester_name = msg.get('requester_name', '')
        requester_instance = msg.get('requester_instance', '')

        if sign_id not in signs:
            send_json(conn, {'type': 'request_summon_ack', 'success': False,
                             'error': '标记不存在或已过期'})
            return None

        owner_instance = signs[sign_id].get('instance_id')
        if owner_instance not in instances:
            send_json(conn, {'type': 'request_summon_ack', 'success': False,
                             'error': '放置者已离线'})
            return None

        signs[sign_id]['state'] = 'BeingSummoned'

        owner_conn = instances[owner_instance]['conn']
        send_json(owner_conn, {
            'type': 'summon_request',
            'sign_id': sign_id,
            'requester_name': requester_name,
            'requester_instance': requester_instance,
            'requester_level': msg.get('requester_level', 0),
        })

        send_json(conn, {'type': 'request_summon_ack', 'success': True})
        print(f"[召唤] {requester_instance}({requester_name}) "
              f"请求召唤标记 {sign_id[:8]}...")
        return None

    elif msg_type == 'accept_summon':
        sign_id = msg.get('sign_id')
        requester_instance = msg.get('requester_instance')

        if requester_instance in instances:
            requester_conn = instances[requester_instance]['conn']
            send_json(requester_conn, {
                'type': 'summon_accepted',
                'sign_id': sign_id,
            })

        if sign_id in signs:
            del signs[sign_id]

        print(f"[召唤] 标记 {sign_id[:8]}... 被接受")
        return None

    elif msg_type == 'decline_summon':
        sign_id = msg.get('sign_id')
        requester_instance = msg.get('requester_instance')

        if requester_instance in instances:
            requester_conn = instances[requester_instance]['conn']
            send_json(requester_conn, {
                'type': 'summon_declined',
                'sign_id': sign_id,
            })

        if sign_id in signs:
            signs[sign_id]['state'] = 'Active'

        print(f"[召唤] 标记 {sign_id[:8]}... 被拒绝")
        return None

    # === 4. 灵体数据传输 ===
    elif msg_type == 'transfer_phantom_data':
        target_instance = msg.get('target_instance')
        if target_instance in instances:
            target_conn = instances[target_instance]['conn']
            send_json(target_conn, {
                'type': 'phantom_data_received',
                'data': msg.get('data', {}),
            })
            print(f"[数据] 灵体数据已转发到 {target_instance}")
        else:
            print(f"[错误] 转发失败: 实例 {target_instance} 不在线")
        return None

    elif msg_type == 'ping':
        send_json(conn, {'type': 'pong'})
        return None

    else:
        print(f"[警告] 未知消息类型: {msg_type}")
        send_json(conn, {'type': 'error', 'message': f'未知类型: {msg_type}'})
        return None


def start_server():
    """启动TCP服务器"""
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    try:
        server.bind((HOST, PORT))
    except OSError as e:
        print(f"[错误] 端口 {PORT} 被占用: {e}")
        sys.exit(1)

    server.listen(10)
    print(f"╔═══════════════════════════════════════╗")
    print(f"║   SoulLike 中间匹配服务已启动          ║")
    print(f"║   监听: {HOST}:{PORT}                   ║")
    print(f"╚═══════════════════════════════════════╝")

    try:
        while True:
            conn, addr = server.accept()
            thread = threading.Thread(target=handle_client, args=(conn, addr))
            thread.daemon = True
            thread.start()
    except KeyboardInterrupt:
        print("\n[关闭] 服务停止")
    finally:
        server.close()


if __name__ == '__main__':
    start_server()
