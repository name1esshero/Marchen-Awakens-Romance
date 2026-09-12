#!/usr/bin/env python3
"""Local map editor; serve only the UI and validated project APIs."""
import argparse
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
from pathlib import Path
import secrets
import threading
import webbrowser
import struct
import sys
from urllib.parse import unquote, urlsplit
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from map_editor.model import Project

STATIC = Path(__file__).resolve().parent


def public(data):
    return {k:v for k,v in data.items() if not k.startswith('_')}


def make_server(project, port=8765):
    token=secrets.token_urlsafe(32)
    lock=threading.RLock()
    class Handler(BaseHTTPRequestHandler):
        def log_message(self, fmt, *args):
            pass

        def reply(self, status, data, mime='application/json'):
            raw=json.dumps(data).encode() if mime=='application/json' else data
            self.send_response(status)
            for key,value in {'Content-Type':mime,'Content-Length':str(len(raw)),
                              'Cache-Control':'no-store','X-Content-Type-Options':'nosniff',
                              'Referrer-Policy':'no-referrer',
                              'Content-Security-Policy':"default-src 'self'; img-src 'self' data:; frame-ancestors 'none'"}.items():
                self.send_header(key,value)
            self.end_headers();self.wfile.write(raw)

        def allowed(self, api=False):
            origins={f'http://127.0.0.1:{self.server.server_port}',f'http://localhost:{self.server.server_port}'}
            if 'http://'+self.headers.get('Host','') not in origins:
                self.reply(403,{'error':'Invalid editor host'});return False
            origin=self.headers.get('Origin')
            if origin and origin not in origins:
                self.reply(403,{'error':'Foreign origin refused'});return False
            if api and not secrets.compare_digest(self.headers.get('X-Editor-Token',''),token):
                self.reply(403,{'error':'Open the editor URL printed by make map-editor'});return False
            return True

        def do_GET(self):
            path=urlsplit(self.path).path
            if not self.allowed(path.startswith('/api/')):return
            try:
                with lock:
                    if path=='/api/catalog':return self.reply(200,project.catalog())
                    if path.startswith('/api/map/'):
                        return self.reply(200,public(project.load(unquote(path[len('/api/map/'):]))))
                    if path.startswith('/api/script/'):
                        return self.reply(200,project.script_data(unquote(path[len('/api/script/'):])) )
                files={'/':('index.html','text/html; charset=utf-8'),'/app.js':('app.js','text/javascript; charset=utf-8'),'/style.css':('style.css','text/css; charset=utf-8')}
                if path not in files:return self.reply(404,{'error':'Not found'})
                name,mime=files[path];self.reply(200,(STATIC/name).read_bytes(),mime)
            except (ValueError,KeyError,TypeError,UnicodeError,IndexError,struct.error) as ex:self.reply(400,{'error':str(ex)})
            except OSError as ex:self.reply(500,{'error':str(ex)})

        def do_POST(self):
            if not self.allowed(True):return
            path=urlsplit(self.path).path
            if not path.startswith('/api/map/'):return self.reply(404,{'error':'Not found'})
            try:
                size=int(self.headers.get('Content-Length','0'))
                if not 0<size<=8*1024*1024:raise ValueError('Invalid request length')
                if self.headers.get_content_type()!='application/json':raise ValueError('Expected JSON')
                payload=json.loads(self.rfile.read(size))
                with lock:
                    data=project.save(unquote(path[len('/api/map/'):]),payload)
                    if payload.get('script'):data['saved_script']=project.script_data(payload['script']['name'])
                self.reply(200,public(data))
            except (ValueError,KeyError,TypeError,UnicodeError,IndexError,struct.error) as ex:
                self.reply(409 if 'changed on disk' in str(ex) else 400,{'error':str(ex)})
            except OSError as ex:self.reply(500,{'error':str(ex)})
    server=ThreadingHTTPServer(('127.0.0.1',port),Handler)
    return server,token


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--port',type=int,default=8765)
    parser.add_argument('--no-browser',action='store_true',help='Print the editor URL without opening a browser')
    args=parser.parse_args();server,token=make_server(Project(),args.port)
    url=f'http://127.0.0.1:{server.server_port}/#token={token}'
    print(f'Open {url}',flush=True)
    print('Ctrl+C stops the editor. Saves write editable JSON sources; run make to rebuild.',flush=True)
    if not args.no_browser:
        def open_browser():
            try:
                if webbrowser.open(url,new=2):return
            except (OSError,webbrowser.Error):pass
            print('Could not open a browser automatically. Open the URL above manually.',flush=True)
        # The socket is already listening. Keep browser launch off the request
        # thread so a browser waiting for its first response cannot block it.
        threading.Thread(target=open_browser,daemon=True).start()
    try:server.serve_forever()
    except KeyboardInterrupt:pass
    finally:server.server_close()


if __name__=='__main__':main()
