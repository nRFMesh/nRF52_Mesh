var camera, scene, renderer;
var mesh,wiremesh;
var node1;

var container,controls;

var MyHome;
var nodes_config;
var house;


import {NodesTable} from './nodes_table.js';
import { STLLoader } from '../js/STLLoader.js';

$.getJSON("nodes.json", function(json) {
	nodes_config = json;
	console.log("loaded sensors config");
	$.getJSON("house.json", function(house_json) {
		house = house_json;
		console.log("loaded house config");
		init();
		animate();
	
	
	});
});

function swap_yz(pos){
	return new THREE.Vector3(pos.x,pos.z,-pos.y);
}

class Node{
	constructor(id){
		this.id = id;
		var size = 20;
		var nb_sections = 64;
		var geometry = new THREE.SphereGeometry( size, nb_sections,nb_sections );
		//material = new THREE.MeshBasicMaterial( {color: 0xffff00} );
		var material = new THREE.MeshPhongMaterial( {
			color: 0x156289,
			emissive: 0x072534,
			side: THREE.DoubleSide,
			flatShading: true
		});

		this.mesh = new THREE.Mesh( geometry, material );

		var wiregeom = new THREE.SphereBufferGeometry( size, 6, 5 );
		//material = new THREE.MeshBasicMaterial( {color: 0xffff00} );
		var wirematerial = new THREE.MeshBasicMaterial( {wireframe:true, color:0x000000} );

		this.wiremesh = new THREE.Mesh( wiregeom, wirematerial );

		
		var pos;
		if("position" in nodes_config[id])
		{
			pos = nodes_config[id].position;
			pos = swap_yz(pos);
			console.log("nodeid "+id+" has position : "+pos.x,pos.y,pos.z);
		}
		else
		{
			pos = {"x":0,"y":0,"z":0}
			console.log("nodeid "+id+" has no position");
		}

		this.mesh.position.set(pos.x,pos.y,pos.z);
		this.wiremesh.position.set(pos.x,pos.y,pos.z);

		//scene.add( this.wiremesh );
		scene.add( this.mesh );
	}
}

class Home {
	constructor(nodes_array) {
	  this.nodes = nodes_array;
	}

	add_node(id){
		if(this.nodes.includes(id))
		{
			console.log("Node already available")
		}
		else
		{
			var node = new Node(id);
			this.nodes.push(id);
			console.log(this.nodes);
			NodesTable.addNodeToTable(nodes_config[id].name,id);
			console.log("On Home Added Node id > "+id);
		}
	}

	on_message(mqtt_post){
		var vals = mqtt_post.split("/");
		if(vals.length == 3)
		{
			var nodeid = vals[1];
			this.add_node(parseInt(nodeid));
		}
	}

}

function lights(model){
	for (const [room_name,room] of Object.entries(model.Rooms)) {
		var light = new THREE.PointLight( 0xffffff, 1, 800, 2 );
		light.position.set( room.center.x, 200, room.center.y );
		console.log("light set in ",room_name," at ",room.center.x,",",room.center.y);
		scene.add( light );
	}
}

class Plane{
	static init(w,h) {
		var plane = new THREE.PlaneGeometry( w,h,4,4 );
		//var pmat = new THREE.MeshBasicMaterial( {color: 0xffff00, side: THREE.DoubleSide} );
		var pmat = new THREE.MeshPhongMaterial( {
			color: 0xABABAB,
			emissive: 0xABABAB,
			side: THREE.DoubleSide,
			flatShading: true
		});
		
		var planemesh = new THREE.Mesh( plane, pmat );
		planemesh.rotation.x = Math.PI / 2;
		scene.add( planemesh );

		plane = new THREE.PlaneBufferGeometry( w,h,4,4 );
		pmat = new THREE.MeshBasicMaterial( {wireframe:true, color:0x000000} );
		
		planemesh = new THREE.Mesh( plane, pmat );
		planemesh.rotation.x = Math.PI / 2;
		scene.add( planemesh );
	}
}

function center_mesh(mesh){
	var box = new THREE.Box3().setFromObject( mesh );
	//console.log( "boudning box : ",box.min, box.max, box.getSize() );
	var box_center = box.getCenter();
	mesh.position.set( -box_center.x, 0, -box_center.z );
	//console.log("tx : ",-box_center.x, " ty : ", -box_center.z );
}

class STLModel{
	static init() {
		var material = new THREE.MeshPhongMaterial( { color: 0xFFFFFF, specular: 0x111111, shininess: 10 } );

		var loader = new STLLoader();
		loader.load( '../models/Valery_Open.stl', function ( geometry ) {
			var mesh = new THREE.Mesh( geometry, material );
			center_mesh(mesh);
			mesh.castShadow = true;
			mesh.receiveShadow = true;
			scene.add( mesh );

		} );
	}
}

class RoomName{
	static add(name, pos_x,pos_z) {
		var loader = new THREE.FontLoader();

		loader.load( 'fonts/helvetiker_regular.typeface.json', function ( font ) {
		
			var material = new THREE.MeshPhongMaterial( { color: 0xAAAAEA, specular: 0x111171, shininess: 200 } );
			var geometry = new THREE.TextGeometry( name, {
				font: font,
				size: 40,
				height: 5,
				curveSegments: 12,
				bevelEnabled: true,
				bevelThickness: 8,
				bevelSize: 4,
				bevelOffset: 0,
				bevelSegments: 5
			} );
			var mesh = new THREE.Mesh( geometry, material );
			mesh.castShadow = true;
			mesh.receiveShadow = true;
			//mesh.rotation.set(new THREE.Vector3(Math.PI / 2, 0, Math.PI));
			mesh.rotation.x = -Math.PI / 2;
			//mesh.rotation.z = Math.PI;
			mesh.position.set(pos_x, 20, pos_z );
			scene.add( mesh );
		} );
	}
}


function add_controls(){
	controls = new THREE.OrbitControls( camera, renderer.domElement );

	//controls.addEventListener( 'change', render ); // call this only in static scenes (i.e., if there is no animation loop)

	controls.enableDamping = true; // an animation loop is required when either damping or auto-rotation are enabled
	controls.dampingFactor = 1.5;//0.1:too rolly, 1: smooth, 2 unstable

	controls.screenSpacePanning = false;

	controls.minDistance = 100;
	controls.maxDistance = 10000;

	controls.minPolarAngle =  10 * Math.PI / 180;
	controls.maxPolarAngle =  80 * Math.PI / 180;

	controls.rotateSpeed = 0.7;

}

function onWindowResize() {

	var w = container.clientWidth;
	var h = container.clientHeight;
	camera.aspect = w / h;
	camera.updateProjectionMatrix();

	renderer.setSize( w, h );

}


function init() {

	console.log("===> inside init()");
	if ( ! Detector.webgl ) Detector.addGetWebGLMessage();

	container = document.getElementById('viewer');
	var w = container.clientWidth;
	var h = container.clientHeight;
	
	camera = new THREE.PerspectiveCamera( 45, w / h, 10, 2000 );
	camera.position.z = 1000;
	camera.position.y = 300;

	scene = new THREE.Scene();

	STLModel.init();
	MyHome = new Home([]);
	for (const [room_name,room] of Object.entries(house.Rooms)) {
		console.log("Added Room name : ",room_name);
		RoomName.add(room_name,room.center.x,room.center.y);
	}
	for(var i=0;i< house.Sensors.length;i++){
			MyHome.add_node(house.Sensors[i]);
	}
	lights(house);


	renderer = new THREE.WebGLRenderer( { antialias: true,alpha:true } );
	renderer.setSize( w, h );
	renderer.setClearColor( 0x000000, 0.0 );

	container.appendChild(renderer.domElement);

	add_controls();

	window.addEventListener( 'resize', onWindowResize, false );

}

function animate() {

	requestAnimationFrame( animate );

	controls.update(); // only required if controls.enableDamping = true, or if controls.autoRotate = true

	renderer.render( scene, camera );

}

export {MyHome};
//export{init}
