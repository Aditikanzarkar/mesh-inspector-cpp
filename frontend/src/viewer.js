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

  console.log('[viewer] init', {
    clientWidth: container.clientWidth,
    clientHeight: container.clientHeight,
    rect: container.getBoundingClientRect?.(),
  });

  scene.add(new THREE.AmbientLight(0xffffff, 0.65));
  const light = new THREE.DirectionalLight(0xffffff, 0.85);
  light.position.set(3, 5, 2);
  scene.add(light);

  const grid = new THREE.GridHelper(10, 10, 0x415068, 0x2b3444);
  scene.add(grid);

  let mesh = null;
  const loader = new STLLoader();

  function setMeshFromArrayBuffer(buffer) {
    console.log('[viewer] parsing STL', { bytes: buffer.byteLength });
    const geometry = loader.parse(buffer);
    geometry.computeVertexNormals();
    geometry.computeBoundingBox();

    if (mesh) {
      scene.remove(mesh);
      mesh.geometry.dispose();
      mesh.material.dispose();
    }

    const material = new THREE.MeshStandardMaterial({
      color: 0x7ab7ff,
      metalness: 0.1,
      roughness: 0.8,
    });

    mesh = new THREE.Mesh(geometry, material);

    const box = geometry.boundingBox;
    const center = new THREE.Vector3();
    box.getCenter(center);
    mesh.position.sub(center);

    scene.add(mesh);

    const size = box.getSize(new THREE.Vector3()).length() || 1;
    camera.position.set(size, size, size);
    controls.target.set(0, 0, 0);
    controls.update();

    const pos = geometry.getAttribute?.('position');
    const triCount = pos ? Math.floor(pos.count / 3) : null;
    console.log('[viewer] loaded geometry', { triCount, size });
  }

  function onResize() {
    const { clientWidth, clientHeight } = container;
    if (!clientWidth || !clientHeight) return;
    camera.aspect = clientWidth / clientHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(clientWidth, clientHeight);
  }

  window.addEventListener('resize', onResize);
  const ro = new ResizeObserver(onResize);
  ro.observe(container);
  queueMicrotask(onResize);

  function animate() {
    controls.update();
    renderer.render(scene, camera);
    requestAnimationFrame(animate);
  }

  animate();

  return { setMeshFromArrayBuffer };
}
