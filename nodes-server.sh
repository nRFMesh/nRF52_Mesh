docker run --name nodes-server -v /home/pi/nRF52_Mesh/www:/usr/share/nginx/html:ro -p 8080:80 -d --restart unless-stopped nginx
