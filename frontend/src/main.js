import { createViewer } from './viewer.js';
import { bindUploader } from './uploader.js';
import { setStatus } from './ui.js';

const viewerEl = document.getElementById('viewer');
const inputEl = document.getElementById('stl-input');

const viewer = createViewer(viewerEl);

bindUploader(
  inputEl,
  (buffer, name) => {
    setStatus(`Loading: ${name}`);
    try {
      viewer.setMeshFromArrayBuffer(buffer);
      setStatus(`Loaded: ${name}`);
    } catch {
      setStatus('Failed to parse STL.');
    }
  },
  () => setStatus('Failed to load STL file.')
);
