export async function fetchMeshMetrics(stlFile) {
  const response = await fetch('/api/mesh-metrics', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/octet-stream',
    },
    body: stlFile,
  });

  const payload = await response.json();

  if (!response.ok) {
    throw new Error(payload?.error || 'Failed to compute mesh metrics.');
  }

  return payload;
}
