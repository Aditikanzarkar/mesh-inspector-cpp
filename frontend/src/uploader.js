export function bindUploader(inputEl, onFile, onError) {
  inputEl.addEventListener('change', async () => {
    const file = inputEl.files?.[0];
    if (!file) return;

    try {
      console.log('[uploader] selected', {
        name: file.name,
        size: file.size,
        type: file.type,
      });
      const buffer = await file.arrayBuffer();
      console.log('[uploader] read arrayBuffer', { bytes: buffer.byteLength });
      onFile(buffer, file.name);
    } catch (err) {
      console.error('[uploader] failed', err);
      onError?.(err);
    }
  });
}
