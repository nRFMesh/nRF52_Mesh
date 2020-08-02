sudo cp raspi/rf_uart/mesh_controller.service /lib/systemd/system/
sudo chmod 644 /lib/systemd/system/mesh_controller.service
sudo chmod +x raspi/rf_uart/mesh_controller.py
sudo systemctl daemon-reload
sudo systemctl enable mesh_controller.service
sudo systemctl start mesh_controller.service

