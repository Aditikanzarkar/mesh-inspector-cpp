export function setStatus(message) {
  const statusEl = document.getElementById('status');
  if (statusEl) statusEl.textContent = message;
}

export function setMetrics(metrics) {
  const areaEl = document.getElementById('metric-area');
  const volumeEl = document.getElementById('metric-volume');
  const trianglesEl = document.getElementById('metric-triangles');

  if (!areaEl || !volumeEl || !trianglesEl) return;

  if (!metrics) {
    areaEl.textContent = '-';
    volumeEl.textContent = '-';
    trianglesEl.textContent = '-';
    return;
  }

  areaEl.textContent = Number(metrics.area).toFixed(6);
  volumeEl.textContent = Number(metrics.volume).toFixed(6);
  trianglesEl.textContent = String(metrics.triangles);
}
