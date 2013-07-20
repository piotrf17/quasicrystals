// Quasicrystals, implemented as a GL fragment shader.
// Requires OpenGL 2.0 or greater.

const float kPi = 3.14159;
const float kWidth = 400.0;
const float kHeight = 400.0;
const float kFreq = 1.0 / 5.0;
const int kNumWaves = 7;

void main() {
  float x = gl_FragCoord.x - 0.5 * kWidth;
  float y = gl_FragCoord.y - 0.5 * kHeight;

  // Initialize angles sines and coses.
  float coses[kNumWaves];
  float sines[kNumWaves];
  for (int i = 0; i < kNumWaves; ++i) {
    float angle = float(i) * kPi / float(kNumWaves);
    coses[i] = cos(angle);
    sines[i] = sin(angle);
  }

  // TODO(piotrf): Make this variable.
  float step = 1.0;

  float p = 0.0;
  for (int w = 0; w < kNumWaves; ++w) {
    float cx = coses[w] * x;
    float sy = sines[w] * y;
    float phase = step * 0.05 * (float(w) + 1.0);
    p += 0.5 * (cos(kFreq * (cx + sy) + phase) + 1.0);
  }

  p = 0.5 * (cos(kPi * p) + 1.0);

  gl_FragColor = vec4(p, p, p, 1.0);
}
