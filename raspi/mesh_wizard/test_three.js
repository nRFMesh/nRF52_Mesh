var camera, scene, renderer;
var geometry, material, mesh,wiremesh;

var container;

init();
animate();


function geometries(){

	geometry = new THREE.SphereGeometry( 0.2, 6, 5 );
	//material = new THREE.MeshBasicMaterial( {color: 0xffff00} );
	material = new THREE.MeshPhongMaterial( {
		color: 0x156289,
		emissive: 0x072534,
		side: THREE.DoubleSide,
		flatShading: true
	});

	mesh = new THREE.Mesh( geometry, material );

	var wiregeom = new THREE.SphereBufferGeometry( 0.2, 6, 5 );
	//material = new THREE.MeshBasicMaterial( {color: 0xffff00} );
	var wirematerial = new THREE.MeshBasicMaterial( {wireframe:true, color:0x000000} );

	wiremesh = new THREE.Mesh( wiregeom, wirematerial );

	mesh.position.set(0,0.3,0);
	wiremesh.position.set(0,0.3,0);

	scene.add( wiremesh );
	scene.add( mesh );

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

function plane1(){
	var plane = new THREE.Plane( new THREE.Vector3( 0, 1, 0 ), 0 );
	var helper = new THREE.PlaneHelper( plane, 1, 0xffff00 );
	scene.add( helper );	
}

function plane2(){
	var plane = new THREE.PlaneGeometry( 2,2,4,4 );
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

	plane = new THREE.PlaneBufferGeometry( 2,2,4,4 );
	pmat = new THREE.MeshBasicMaterial( {wireframe:true, color:0x000000} );
	
	planemesh = new THREE.Mesh( plane, pmat );
	planemesh.rotation.x = Math.PI / 2;
	scene.add( planemesh );
}

function init() {

	container = document.getElementById('viewer');
	var w = container.clientWidth;
	var h = container.clientHeight;
	console.log("w:",w);
	console.log("h:",h);
	camera = new THREE.PerspectiveCamera( 70, w / h, 0.01, 10 );
	camera.position.z = 2;
	camera.position.y = 0.6;
	scene = new THREE.Scene();

	geometries();

	plane2();

	lights();


	renderer = new THREE.WebGLRenderer( { antialias: true } );
	renderer.setSize( w, h );
	renderer.setClearColor( 0x000000, 1 );
	//document.body.appendChild( renderer.domElement );
	container.appendChild(renderer.domElement);

}

function animate() {

	requestAnimationFrame( animate );

	//mesh.rotation.x += 0.01;
	mesh.rotation.y += 0.02;
	wiremesh.rotation.y += 0.02;

	renderer.render( scene, camera );

}