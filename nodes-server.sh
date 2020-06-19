docker run --name nodes-server -v /home/pi/nRF52_Mesh/www/nodes.json:/usr/share/nginx/html/nodes.json:ro -p 8080:80 -d --restart unless-stopped nginx
