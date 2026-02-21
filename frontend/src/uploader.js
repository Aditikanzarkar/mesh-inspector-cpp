export function bindUploader(inputEl, onFile, onError) {
  inputEl.addEventListener('change', async () => {
    const file = inputEl.files?.[0];
    if (!file) return;

    try {
      const buffer = await file.arrayBuffer();
      onFile(buffer, file.name);
    } catch (err) {
      onError?.(err);
    }
  });
}
