#!/bin/bash
sudo rm -rf /var/www/html/mesh
sudo cp -r mesh_wizard /var/www/html/mesh
sudo rm /var/www/html/mesh/deploy.sh
