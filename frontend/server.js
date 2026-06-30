import http from 'node:http';
import { randomBytes } from 'node:crypto';
import { spawn } from 'node:child_process';
import { createWriteStream, promises as fs } from 'node:fs';
import path from 'node:path';
import os from 'node:os';
import { fileURLToPath } from 'node:url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const HOST = process.env.HOST || '127.0.0.1';
const PORT = Number(process.env.PORT || 5173);

const MIME_TYPES = {
  '.html': 'text/html; charset=utf-8',
  '.js': 'text/javascript; charset=utf-8',
  '.css': 'text/css; charset=utf-8',
  '.json': 'application/json; charset=utf-8',
  '.svg': 'image/svg+xml',
  '.png': 'image/png',
  '.jpg': 'image/jpeg',
  '.jpeg': 'image/jpeg',
};

function sendJson(res, statusCode, payload) {
  const body = JSON.stringify(payload);
  res.writeHead(statusCode, {
    'Content-Type': 'application/json; charset=utf-8',
    'Content-Length': Buffer.byteLength(body),
  });
  res.end(body);
}

function streamRequestToFile(req, filePath, maxBytes = 100 * 1024 * 1024) {
  return new Promise((resolve, reject) => {
    const writeStream = createWriteStream(filePath);
    let size = 0;
    let settled = false;

    const cleanup = () => {
      req.off('data', onData);
      req.off('end', onEnd);
      req.off('error', onError);
      req.off('aborted', onAborted);
      writeStream.off('error', onError);
      writeStream.off('finish', onFinish);
    };

    const finish = (err) => {
      if (settled) return;
      settled = true;
      cleanup();
      if (err) reject(err);
      else resolve(size);
    };

    const onData = (chunk) => {
      size += chunk.length;
      if (size > maxBytes) {
        writeStream.destroy();
        req.destroy();
        finish(new Error('Uploaded STL exceeds size limit (100MB).'));
      }
    };

    const onEnd = () => {
      writeStream.end();
    };

    const onAborted = () => {
      writeStream.destroy();
      finish(new Error('Request was aborted before upload completed.'));
    };

    const onError = (err) => {
      finish(err);
    };

    const onFinish = () => {
      finish();
    };

    req.on('data', onData);
    req.on('end', onEnd);
    req.on('error', onError);
    req.on('aborted', onAborted);
    writeStream.on('error', onError);
    writeStream.on('finish', onFinish);

    req.pipe(writeStream, { end: false });
  });
}

function parseMeshInspectorOutput(stdout) {
  const triangles = stdout.match(/triangles:\s*([0-9]+)/i);
  const area = stdout.match(/total area:\s*([-+0-9.eE]+)/i);
  const volume = stdout.match(/estimated volume:\s*([-+0-9.eE]+)/i);

  if (!triangles || !area || !volume) {
    throw new Error('Unexpected executable output format.');
  }

  return {
    triangles: Number(triangles[1]),
    area: Number(area[1]),
    volume: Number(volume[1]),
  };
}

async function resolveExecutablePath() {
  if (process.env.MESH_INSPECTOR_EXE) {
    return process.env.MESH_INSPECTOR_EXE;
  }

  const repoRoot = path.resolve(__dirname, '..');
  const candidateDirs = [
    path.join(repoRoot, 'bazel-bin', 'backend'),
    path.join(repoRoot, 'bazel-bin'),
    path.join(repoRoot, 'build', 'backend'),
    path.join(repoRoot, 'build'),
  ];
  const executableNames = process.platform === 'win32'
    ? ['mesh_inspector.exe', 'mesh_inspector']
    : ['mesh_inspector'];

  for (const dir of candidateDirs) {
    for (const name of executableNames) {
      const candidate = path.join(dir, name);
      try {
        await fs.access(candidate);
        return candidate;
      } catch {
        // Continue trying candidates.
      }
    }
  }

  throw new Error(
    'mesh_inspector executable not found. Build it with: bazel build //backend:mesh_inspector'
  );
}

function runMeshInspector(executablePath, stlPath) {
  return new Promise((resolve, reject) => {
    const child = spawn(executablePath, [stlPath], {
      stdio: ['ignore', 'pipe', 'pipe'],
    });

    let stdout = '';
    let stderr = '';

    child.stdout.on('data', (chunk) => {
      stdout += chunk.toString();
    });

    child.stderr.on('data', (chunk) => {
      stderr += chunk.toString();
    });

    child.on('error', reject);
    child.on('close', (code) => {
      if (code !== 0) {
        reject(new Error(stderr.trim() || `mesh_inspector failed with code ${code}`));
        return;
      }

      try {
        resolve(parseMeshInspectorOutput(stdout));
      } catch (err) {
        reject(err);
      }
    });
  });
}

async function handleMetricsApi(req, res) {
  let tempPath = '';

  try {
    const executablePath = await resolveExecutablePath();

    const tempName = `mesh-inspector-${randomBytes(16).toString('hex')}.stl`;
    tempPath = path.join(os.tmpdir(), tempName);

    const uploadedBytes = await streamRequestToFile(req, tempPath);
    if (!uploadedBytes) {
      sendJson(res, 400, { error: 'No STL content provided.' });
      return;
    }

    const metrics = await runMeshInspector(executablePath, tempPath);
    sendJson(res, 200, metrics);
  } catch (err) {
    const message = err instanceof Error ? err.message : 'Failed to compute mesh metrics.';
    const statusCode = message.includes('exceeds size limit') ? 413 : 500;
    sendJson(res, statusCode, { error: message });
  } finally {
    if (!tempPath) return;
    try {
      await fs.unlink(tempPath);
    } catch (unlinkErr) {
      if (!unlinkErr || unlinkErr.code !== 'ENOENT') {
        console.error('Failed to clean up temp STL:', unlinkErr);
      }
    }
  }
}

async function serveStatic(req, res) {
  let requestedPath = req.url || '/';
  if (requestedPath === '/') requestedPath = '/index.html';

  const safePath = path.normalize(requestedPath).replace(/^([.][.][/\\])+/, '');
  const filePath = path.resolve(__dirname, `.${safePath}`);

  if (!filePath.startsWith(__dirname)) {
    res.writeHead(403);
    res.end('Forbidden');
    return;
  }

  try {
    const data = await fs.readFile(filePath);
    const ext = path.extname(filePath).toLowerCase();
    const mime = MIME_TYPES[ext] || 'application/octet-stream';

    res.writeHead(200, { 'Content-Type': mime });
    res.end(data);
  } catch {
    res.writeHead(404);
    res.end('Not found');
  }
}

const server = http.createServer(async (req, res) => {
  if (!req.url || !req.method) {
    res.writeHead(400);
    res.end('Bad request');
    return;
  }

  if (req.method === 'POST' && req.url === '/api/mesh-metrics') {
    await handleMetricsApi(req, res);
    return;
  }

  if (req.method === 'GET') {
    await serveStatic(req, res);
    return;
  }

  res.writeHead(405);
  res.end('Method not allowed');
});

server.listen(PORT, HOST, () => {
  console.log(`Mesh Inspector frontend+API running at http://${HOST}:${PORT}`);
});
