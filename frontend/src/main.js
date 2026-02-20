import { createViewer } from './viewer.js';
import { bindUploader } from './uploader.js';
import { setStatus } from './ui.js';

const viewerEl = document.getElementById('viewer');
const inputEl = document.getElementById('stl-input');

console.log('[main] boot', {
  viewerEl: !!viewerEl,
  inputEl: !!inputEl,
  viewerRect: viewerEl?.getBoundingClientRect?.(),
});

const viewer = createViewer(viewerEl);

bindUploader(
  inputEl,
  (buffer, name) => {
    setStatus(`Loading: ${name}`);
    try {
      viewer.setMeshFromArrayBuffer(buffer);
      setStatus(`Loaded: ${name}`);
    } catch (err) {
      console.error('[main] setMesh failed', err);
      setStatus('Failed to parse STL (see Console).');
    }
  },
  () => setStatus('Failed to load STL file.')
);
