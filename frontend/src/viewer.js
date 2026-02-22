import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import { STLLoader } from 'three/addons/loaders/STLLoader.js';

export function createViewer(container) {
  const scene = new THREE.Scene();
  scene.background = new THREE.Color(0x0f1115);

  const camera = new THREE.PerspectiveCamera(
    60,
    container.clientWidth / container.clientHeight,
    0.1,
    2000
  );
  camera.position.set(2, 2, 2);

  const renderer = new THREE.WebGLRenderer({ antialias: true });
  renderer.setPixelRatio(window.devicePixelRatio);
  renderer.setSize(container.clientWidth, container.clientHeight);
  container.appendChild(renderer.domElement);

  const controls = new OrbitControls(camera, renderer.domElement);
  controls.enableDamping = true;

  scene.add(new THREE.AmbientLight(0xffffff, 0.65));
  const light = new THREE.DirectionalLight(0xffffff, 0.85);
  light.position.set(3, 5, 2);
  scene.add(light);

  const grid = new THREE.GridHelper(10, 10, 0x415068, 0x2b3444);
  scene.add(grid);

  let mesh = null;
  let triangleEdges = null;
  const loader = new STLLoader();

  function setMeshFromArrayBuffer(buffer) {
    const geometry = loader.parse(buffer);
    geometry.computeVertexNormals();
    geometry.computeBoundingBox();

    if (mesh) {
      scene.remove(mesh);
      mesh.geometry.dispose();
      mesh.material.dispose();
    }
    if (triangleEdges) {
      scene.remove(triangleEdges);
      triangleEdges.geometry.dispose();
      triangleEdges.material.dispose();
    }

    const material = new THREE.MeshStandardMaterial({
      color: 0x7ab7ff,
      metalness: 0.1,
      roughness: 0.8,
    });

    mesh = new THREE.Mesh(geometry, material);
    mesh.renderOrder = 0;
    
    // ---- TRIANGLE EDGES ----

    const wireframe = new THREE.WireframeGeometry(geometry);

    const edgeMaterial = new THREE.LineBasicMaterial({
      color: 0xff0000,      // make it obvious
      opacity: 1,
      depthTest: false,     // VERY IMPORTANT
      depthWrite: false,
    });
    
    triangleEdges = new THREE.LineSegments(wireframe, edgeMaterial);
    triangleEdges.renderOrder = 1;   // force draw after mesh
    triangleEdges.position.copy(mesh.position);
    
    //scene.add(triangleEdges);
    scene.add(mesh);

    const box = geometry.boundingBox;
    const center = new THREE.Vector3();
    box.getCenter(center);
    mesh.position.sub(center);

    const size = box.getSize(new THREE.Vector3()).length() || 1;
    camera.position.set(size, size, size);
    camera.near = size / 100;
    camera.far = size * 10;
    camera.updateProjectionMatrix();
    controls.target.set(0, 0, 0);
    controls.update();
  }

  function onResize() {
    const { clientWidth, clientHeight } = container;
    if (!clientWidth || !clientHeight) return;
    camera.aspect = clientWidth / clientHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(clientWidth, clientHeight);
  }

  function animate() {
    controls.update();
    renderer.render(scene, camera);
    requestAnimationFrame(animate);
  }

  window.addEventListener('resize', onResize);
  const ro = new ResizeObserver(onResize);
  ro.observe(container);
  queueMicrotask(onResize);

  animate();

  return { setMeshFromArrayBuffer };
}
