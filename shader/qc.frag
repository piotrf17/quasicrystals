// Quasicrystals, implemented as a GL fragment shader.
// Requires OpenGL 2.0 or greater.

const float kPi = 3.14159;
const int kMaxNumWaves = 15;

// Parameters set outside of shader.
uniform float t;           // time
uniform vec2 resolution;   // screen resolution
uniform int num_waves;     // number of waves
uniform float freq;        // spatial frequency of waves

void main() {
  float x = gl_FragCoord.x - 0.5 * resolution.x;
  float y = gl_FragCoord.y - 0.5 * resolution.y;

  // Initialize angles sines and coses.
  float coses[kMaxNumWaves]; 
  float sines[kMaxNumWaves];
  for (int i = 0; i < num_waves; ++i) {
    float angle = float(i) * kPi / float(num_waves);
    coses[i] = cos(angle);
    sines[i] = sin(angle);
  }

  // Compute intensity over the sum of waves.
  float p = 0.0;
  for (int w = 0; w < num_waves; ++w) {
    float cx = coses[w] * x;
    float sy = sines[w] * y;
    float phase = t * 0.05 * (float(w) + 1.0);
    p += 0.5 * (cos(freq * (cx + sy) + phase) + 1.0);
  }

  p = 0.5 * (cos(kPi * p) + 1.0);

  gl_FragColor = vec4(p, p, p, 1.0);
}
