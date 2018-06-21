var camera, scene, renderer;
var mesh,wiremesh;
var node1;

var container,controls;

class Mesh {
	constructor(height, width) {
	  this.height = height;
	  this.width = width;
	}
  }

class Node{
	constructor(pos){
		var size = 0.05;
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

		this.mesh.position.set(pos.x,pos.y,pos.z);
		this.wiremesh.position.set(pos.x,pos.y,pos.z);

		//scene.add( this.wiremesh );
		scene.add( this.mesh );
	}
	rotate(angle){
		this.mesh.rotation.y 		+= angle;
		this.wiremesh.rotation.y 	+= angle;
	}
}

function lights(){
	var lights = [];
	lights[ 0 ] = new THREE.PointLight( 0xffffff, 1, 0 );
	lights[ 1 ] = new THREE.PointLight( 0xffffff, 1, 0 );
	lights[ 2 ] = new THREE.PointLight( 0xffffff, 1, 0 );

	lights[ 0 ].position.set( 0, 200, 0 );
	lights[ 1 ].position.set( 100, 200, 100 );
	lights[ 2 ].position.set( - 100, - 200, - 100 );

	scene.add( lights[ 0 ] );
	scene.add( lights[ 1 ] );
	scene.add( lights[ 2 ] );
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

function controls(){
	controls = new THREE.OrbitControls( camera, renderer.domElement );

	//controls.addEventListener( 'change', render ); // call this only in static scenes (i.e., if there is no animation loop)

	controls.enableDamping = true; // an animation loop is required when either damping or auto-rotation are enabled
	controls.dampingFactor = 1.5;//0.1:too rolly, 1: smooth, 2 unstable

	controls.screenSpacePanning = false;

	controls.minDistance = 1;
	controls.maxDistance = 10;

	controls.minPolarAngle =  10 * Math.PI / 180;
	controls.maxPolarAngle =  80 * Math.PI / 180;

	controls.rotateSpeed = 0.7;

}

function onWindowResize() {

	w = container.clientWidth;
	h = container.clientHeight;
	camera.aspect = w / h;
	camera.updateProjectionMatrix();

	renderer.setSize( w, h );

}


function init() {

	if ( ! Detector.webgl ) Detector.addGetWebGLMessage();

	container = document.getElementById('viewer');
	var w = container.clientWidth;
	var h = container.clientHeight;
	
	camera = new THREE.PerspectiveCamera( 45, w / h, 0.01, 20 );
	camera.position.z = 2;
	camera.position.y = 0.6;

	scene = new THREE.Scene();

	//geometries();
	node1 = new Node(new THREE.Vector3(0,0.2,0));

	node2 = new Node(new THREE.Vector3(0.5,0.2,0));

	//as only one place is needed, no need to create a variable
	Plane.init(2,2);

	lights();


	renderer = new THREE.WebGLRenderer( { antialias: true,alpha:true } );
	renderer.setSize( w, h );
	renderer.setClearColor( 0x000000, 0.0 );

	container.appendChild(renderer.domElement);

	controls();

	window.addEventListener( 'resize', onWindowResize, false );

}

function animate() {

	requestAnimationFrame( animate );

	controls.update(); // only required if controls.enableDamping = true, or if controls.autoRotate = true

	node1.rotate(0.02);

	renderer.render( scene, camera );

}


init();
animate();

