"""
SoulLike 联机召唤系统 — 中间匹配服务
作用：连接多个 UE4 游戏实例，管理召唤标记的注册/查询/转发
包含时序保护：ready_query / phantom_ready

启动：python match_server.py
默认监听 127.0.0.1:7777
"""

import socket
import json
import threading
import uuid
import sys
import re

HOST = '127.0.0.1'
PORT = 7777

instances = {}
signs = {}
conn_owner = {}


def normalize_map(map_name):
    """去除 PIE 模式下的 UEDPIE_N_ 前缀"""
    if map_name and map_name.startswith('UEDPIE_'):
        parts = map_name.split('_', 2)
        if len(parts) == 3:
            return parts[2]
    return map_name


def send_json(conn, data):
    try:
        msg = json.dumps(data, ensure_ascii=False) + '\n'
        conn.sendall(msg.encode('utf-8'))
    except Exception as e:
        print(f"[error] send failed: {e}")


def clean_instance(instance_id):
    if instance_id in instances:
        to_remove = [sid for sid, s in signs.items() if s.get('instance_id') == instance_id]
        for sid in to_remove:
            del signs[sid]
        conn = instances[instance_id]['conn']
        if conn in conn_owner:
            del conn_owner[conn]
        del instances[instance_id]
        print(f"[clean] instance {instance_id} disconnected, removed {len(to_remove)} signs")


def handle_client(conn, addr):
    print(f"[connect] from {addr}")
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
                    print(f"[error] JSON decode failed: {e}")
    except (ConnectionResetError, BrokenPipeError):
        pass
    finally:
        if instance_id:
            clean_instance(instance_id)
        conn.close()


def process_message(conn, msg):
    if not isinstance(msg, dict):
        print(f"[warn] skipping non-dict message: {type(msg).__name__}")
        return None

    msg_type = msg.get('type', '')

    # === 1. 世界实例管理 ===
    if msg_type == 'register_instance':
        instance_id = msg['instance_id']
        instances[instance_id] = {
            'conn': conn,
            'map': normalize_map(msg.get('map', '')),
            'ip': msg.get('ip', ''),
            'port': msg.get('port', 0),
        }
        conn_owner[conn] = instance_id
        print(f"[instance] {instance_id} registered: map={msg.get('map', '')} "
              f"addr={msg.get('ip', '')}:{msg.get('port', 0)}")
        send_json(conn, {'type': 'register_instance_ack', 'success': True})
        return instance_id

    elif msg_type == 'unregister_instance':
        instance_id = conn_owner.get(conn)
        if instance_id:
            clean_instance(instance_id)
            print(f"[instance] {instance_id} unregistered")
        return None

    # === 2. 标记管理 ===
    elif msg_type == 'register_sign':
        instance_id = conn_owner.get(conn)
        if not instance_id:
            send_json(conn, {'type': 'register_sign_ack', 'success': False, 'error': 'instance not registered'})
            return None
        sign_id = str(uuid.uuid4())
        sign_data = msg.get('sign_data', {})
        sign_data['sign_id'] = sign_id
        sign_data['instance_id'] = instance_id
        sign_data['map'] = normalize_map(sign_data.get('map', ''))
        signs[sign_id] = sign_data
        print(f"[sign] {instance_id} placed sign {sign_id[:8]}... total: {len(signs)}")
        send_json(conn, {'type': 'register_sign_ack', 'sign_id': sign_id, 'success': True})
        return None

    elif msg_type == 'unregister_sign':
        sign_id = msg.get('sign_id')
        if sign_id in signs:
            del signs[sign_id]
            print(f"[sign] removed {sign_id[:8]}... remaining: {len(signs)}")
        return None

    elif msg_type == 'query_signs':
        instance_id = conn_owner.get(conn)
        map_name = normalize_map(msg.get('map', ''))
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
            s_copy = dict(s)
            if 'transform' in s_copy and isinstance(s_copy['transform'], dict):
                s_copy['transform'] = json.dumps(s_copy['transform'], ensure_ascii=False)
            results.append(s_copy)
        send_json(conn, {'type': 'query_signs_result', 'signs': results})
        print(f"[query] {instance_id} query: map={map_name}, results={len(results)}")
        return None

    # === 3. 召唤请求 ===
    elif msg_type == 'request_summon':
        sign_id = msg.get('sign_id')
        requester_name = msg.get('requester_name', '')
        requester_instance = msg.get('requester_instance', '')
        requester_level = msg.get('requester_level', 0)
        if sign_id not in signs:
            send_json(conn, {'type': 'request_summon_ack', 'success': False, 'error': 'sign not found'})
            return None
        owner_instance = signs[sign_id].get('instance_id')
        if owner_instance not in instances:
            send_json(conn, {'type': 'request_summon_ack', 'success': False, 'error': 'owner offline'})
            return None
        signs[sign_id]['state'] = 'BeingSummoned'
        owner_conn = instances[owner_instance]['conn']
        requester_info = instances.get(requester_instance, {})
        send_json(owner_conn, {
            'type': 'summon_request',
            'sign_id': sign_id,
            'requester_name': requester_name,
            'requester_instance': requester_instance,
            'requester_ip': requester_info.get('ip', ''),
            'requester_port': requester_info.get('port', 0),
            'requester_level': requester_level,
        })
        send_json(conn, {'type': 'request_summon_ack', 'success': True})
        print(f"[summon] {requester_instance}({requester_name}) requested sign {sign_id[:8]}...")
        return None

    elif msg_type == 'accept_summon':
        sign_id = msg.get('sign_id')
        requester_instance = msg.get('requester_instance')
        if requester_instance in instances:
            requester_conn = instances[requester_instance]['conn']
            send_json(requester_conn, {'type': 'summon_accepted', 'sign_id': sign_id})
        if sign_id in signs:
            del signs[sign_id]
        print(f"[summon] sign {sign_id[:8]}... accepted")
        return None

    elif msg_type == 'decline_summon':
        sign_id = msg.get('sign_id')
        requester_instance = msg.get('requester_instance')
        if requester_instance in instances:
            requester_conn = instances[requester_instance]['conn']
            send_json(requester_conn, {'type': 'summon_declined', 'sign_id': sign_id})
        if sign_id in signs:
            signs[sign_id]['state'] = 'Active'
        print(f"[summon] sign {sign_id[:8]}... declined")
        return None

    # === 4. 灵体数据传输 + 时序保护 ===
    elif msg_type == 'transfer_phantom_data':
        target_instance = msg.get('target_instance')
        if target_instance in instances:
            sender_instance = conn_owner.get(conn)
            target_conn = instances[target_instance]['conn']
            send_json(target_conn, {
                'type': 'phantom_data_received',
                'placer_instance': sender_instance,
                'data': msg.get('data', {}),
            })
            print(f"[data] phantom data forwarded to {target_instance} (from {sender_instance})")
        else:
            print(f"[error] forward failed: instance {target_instance} offline")
        return None

    # === 5. 时序保护 ===
    elif msg_type == 'ready_query':
        session_id = msg.get('session_id')
        target_instance = msg.get('target_instance')
        if target_instance in instances:
            sender_instance = conn_owner.get(conn)
            send_json(instances[target_instance]['conn'], {
                'type': 'ready_query',
                'session_id': session_id,
                'requester_instance': sender_instance,
            })
        else:
            send_json(conn, {'type': 'error', 'message': f'ready_query target {target_instance} offline'})
        return None

    elif msg_type == 'phantom_ready':
        session_id = msg.get('session_id')
        target_instance = msg.get('target_instance')
        if target_instance in instances:
            send_json(instances[target_instance]['conn'], {
                'type': 'phantom_ready',
                'session_id': session_id,
            })
        else:
            send_json(conn, {'type': 'error', 'message': f'phantom_ready target {target_instance} offline'})
        return None

    elif msg_type == 'ping':
        send_json(conn, {'type': 'pong'})
        return None

    else:
        print(f"[warn] unknown message type: {msg_type}")
        send_json(conn, {'type': 'error', 'message': f'unknown type: {msg_type}'})
        return None


def start_server():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    try:
        server.bind((HOST, PORT))
    except OSError as e:
        print(f"[error] port {PORT} in use: {e}")
        sys.exit(1)
    server.listen(10)
    print("============================================")
    print("  SoulLike Match Server")
    print(f"  Listening on {HOST}:{PORT}")
    print("============================================")
    try:
        while True:
            conn, addr = server.accept()
            thread = threading.Thread(target=handle_client, args=(conn, addr))
            thread.daemon = True
            thread.start()
    except KeyboardInterrupt:
        print("\n[shutdown] server stopped")
    finally:
        server.close()


if __name__ == '__main__':
    start_server()
