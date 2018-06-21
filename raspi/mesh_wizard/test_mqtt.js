
var client,textBox;

// called when the client connects
function onConnect() {
  // Once a connection has been made, make a subscription and send a message.
  console.log("onConnect");
  client.subscribe("Nodes/#");
  client.subscribe("jNodes/#");
  client.subscribe("cmd/#");
  client.subscribe("remote_cmd/#");
  /*message = new Paho.MQTT.Message("Hello");
  message.destinationName = "World";
  client.send(message);*/
}

// called when the client loses its connection
function onConnectionLost(responseObject) {
  if (responseObject.errorCode !== 0) {
    console.log("onConnectionLost:"+responseObject.errorMessage);
  }
}

// called when a message arrives
function onMessageArrived(message) {
  console.log(message.destinationName	+ " : "+message.payloadString);
  textBox.value = message.destinationName	+ " : "+message.payloadString+"\r\n"
                  +textBox.value;
  //document.getElementById("meshlog").value += message.destinationName	+ " : "+message.payloadString;
  
}

function setup_buttons(){
  var inNodeId = document.getElementById("inNodeId");
  var btnPing = document.getElementById("btnPing");
  var btnGetChannel = document.getElementById("btnGetChannel");
  var btnRemGetChannel = document.getElementById("btnRemGetChannel");
  btnPing.onclick = function()          { client.send("Nodes/"+inNodeId.value+"/ping","");  }
  btnGetChannel.onclick = function()    { client.send("cmd/request/get_channel","");  }
  btnRemGetChannel.onclick = function() { client.send("remote_cmd/request/get_channel",'{"remote":'+inNodeId.value+'}');  }
}

function main(){
  // Create a client instance
  client = new Paho.MQTT.Client(location.hostname, Number(3000), "clientId");
  // set callback handlers
  client.onConnectionLost = onConnectionLost;
  client.onMessageArrived = onMessageArrived;

  textBox = document.getElementById("meshlog");
  // connect the client
  client.connect({onSuccess:onConnect});
}

//----------------------------------------------------------------------------------

main();

setup_buttons();

