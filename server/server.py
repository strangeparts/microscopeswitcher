from flask import Flask, request, g
from pynput.keyboard import Key, Controller
import applescript
import asyncio
import json
import time
import websockets

loop = asyncio.get_event_loop()
app = Flask(__name__)

keyboard = Controller()

WEBSOCKET_SERVER = '192.168.0.180'

def get_current_scene():
    j = json.dumps({
        'request-type': 'GetCurrentScene',
        'message-id': '2',
    })
    r = loop.run_until_complete(webcall(j))
    j = json.loads(r)
    return j['name']

def switch_scene(scene_name):
    j = json.dumps({
        'request-type': 'SetCurrentScene',
        'message-id': '1',
        'scene-name': scene_name,
    })
    loop.run_until_complete(webcall(j))

async def webcall(j):
    uri = "ws://%s:4444" % WEBSOCKET_SERVER
    async with websockets.connect(uri) as websocket:
        await websocket.send(j)
        response = await websocket.recv()
        print(response)
        return response

@app.route('/switchcamera')
def switchcamera():
    view = request.args.get('view', '')
    if view == "topdown":
        switch_scene(app.previous_scene)
    elif view == "microscope":
        app.previous_scene = get_current_scene()
        switch_scene('Microscope scene')
    return 'view=%s' % view

app.run(host='0.0.0.0')
