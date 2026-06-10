"""
中间匹配服务 — 手动测试脚本
先启动 match_server.py，再跑这个脚本
"""

import socket
import json
import time

HOST = '127.0.0.1'
PORT = 7777


def send_and_recv(conn, msg_dict, label=""):
    msg = json.dumps(msg_dict, ensure_ascii=False) + '\n'
    print(f"\n>>> [{label}] 发送: {msg_dict['type']}")
    conn.sendall(msg.encode('utf-8'))
    time.sleep(0.2)

    response = b""
    while True:
        try:
            chunk = conn.recv(4096)
            if not chunk:
                break
            response += chunk
            if b'\n' in response:
                break
        except:
            break

    if response:
        for line in response.decode('utf-8').strip().split('\n'):
            if line.strip():
                parsed = json.loads(line)
                print(f"<<< [{label}] 收到: {parsed['type']}")


def main():
    print("=== 中间匹配服务 端到端测试 ===")

    connA = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    connA.connect((HOST, PORT))

    connB = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    connB.connect((HOST, PORT))

    # 1. 注册实例
    send_and_recv(connA, {
        "type": "register_instance",
        "instance_id": "InstanceA",
        "map": "地下墓地",
        "ip": "127.0.0.1",
        "port": 17777
    }, "A")

    send_and_recv(connB, {
        "type": "register_instance",
        "instance_id": "InstanceB",
        "map": "地下墓地",
        "ip": "127.0.0.1",
        "port": 17778
    }, "B")

    # 2. 放置标记（A）
    send_and_recv(connA, {
        "type": "register_sign",
        "sign_data": {
            "owner_name": "玩家A",
            "owner_level": 15,
            "weapon_level": 3,
            "map": "地下墓地",
            "transform": '{}',
            "time_remaining": 300
        }
    }, "A")

    # 3. 查询标记（B）
    send_and_recv(connB, {
        "type": "query_signs",
        "map": "地下墓地",
        "level": 15,
        "weapon_level": 2
    }, "B")

    # 4. 请求召唤
    send_and_recv(connB, {
        "type": "request_summon",
        "sign_id": "test-sign-id",
        "requester_name": "玩家B",
        "requester_instance": "InstanceB",
        "requester_level": 14
    }, "B")

    # 5. 传输灵体数据
    send_and_recv(connA, {
        "type": "transfer_phantom_data",
        "target_instance": "InstanceB",
        "data": {
            "owner_name": "玩家A",
            "level": 15,
            "health_percent": 0.8,
            "mesh_path": "/Game/...",
            "equipment": ["sword_01", "shield_02"]
        }
    }, "A")

    connA.close()
    connB.close()
    print("\n=== 测试完成 ===")


if __name__ == '__main__':
    main()
