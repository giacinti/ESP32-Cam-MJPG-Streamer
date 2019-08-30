import os
import time
import urllib.request
import threading
from http.server import BaseHTTPRequestHandler, HTTPServer
from socketserver import ThreadingMixIn

HOST_NAME = '0.0.0.0'
PORT_NUMBER = int(os.getenv("SERVER_PORT",8080))
FPS = int(os.getenv("SERVER_FPS",25))
BOUNDARY = "--boundarydonotcross"

loop_mutex = threading.Lock()

class ThrdHTTPServer(ThreadingMixIn, HTTPServer):
	pass


class MyHandler(BaseHTTPRequestHandler):
        def do_GET(self):
                uri=self.path.split('?')[0] # potentially remove options - octoprint do that
                if uri.endswith('/stream'):
                        self.handle_stream("http:/"+uri.replace("/stream","/image"))
                elif uri.endswith('/image'):
                        self.handle_image("http:/"+uri)
                else:
                        self.send_response(404)

        def send_one_image(self,uri):
                with loop_mutex:
                        req = urllib.request.urlopen(uri)
                        for h in req.info().items():
                                self.send_header(h[0],h[1])
                        self.end_headers()
                        return self.wfile.write(req.read())

        def stream_loop(self,uri):
                while True:
                        try:
                                self.send_one_image(uri)
                                self.wfile.write(bytes(BOUNDARY+"\r\n",'UTF8'))
                                time.sleep(1/FPS)
                        except Exception:
                                return

        def handle_stream(self,uri):
                self.send_response(200)
                self.send_header('Content-type',"multipart/x-mixed-replace; boundary="+BOUNDARY)
                self.end_headers()
                self.flush_headers()
                return self.stream_loop(uri)

        def handle_image(self,uri):
                self.send_response(200)
                return self.send_one_image(uri)

if __name__ == '__main__':
    server_class = ThrdHTTPServer
    httpd = server_class((HOST_NAME, PORT_NUMBER), MyHandler)
    print(time.asctime(), 'Server Starts - %s:%s' % (HOST_NAME, PORT_NUMBER))
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    httpd.server_close()
    print(time.asctime(), 'Server Stops - %s:%s' % (HOST_NAME, PORT_NUMBER))
	
