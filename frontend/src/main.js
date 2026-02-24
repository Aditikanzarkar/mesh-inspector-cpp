import { fetchMeshMetrics } from './api.js';
import { createViewer } from './viewer.js';
import { bindUploader } from './uploader.js';
import { setMetrics, setStatus } from './ui.js';

const viewerEl = document.getElementById('viewer');
const inputEl = document.getElementById('stl-input');

const viewer = createViewer(viewerEl);
setMetrics(null);

bindUploader(
  inputEl,
  async (buffer, name, file) => {
    setStatus(`Loading: ${name}`);
    setMetrics(null);
    const parseStart = performance.now();

    try {
      viewer.setMeshFromArrayBuffer(buffer);
    } catch {
      setStatus('Failed to parse STL.');
      return;
    }
    const parseMs = Math.round(performance.now() - parseStart);

    try {
      const metrics = await fetchMeshMetrics(file);
      setMetrics(metrics);

      const t = metrics.timingsMs;
      if (t) {
        setStatus(
          `Loaded: ${name} | viewer ${parseMs}ms | api ${t.total}ms (read ${t.readUpload} / write ${t.writeTemp} / exe ${t.exeRun})`
        );
        console.log('mesh metrics timings (ms):', t);
      } else {
        setStatus(`Loaded: ${name}`);
      }
    } catch (err) {
      setStatus(err instanceof Error ? err.message : 'Failed to compute metrics.');
    }
  },
  () => setStatus('Failed to load STL file.')
);
