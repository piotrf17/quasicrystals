// Quasicrystals, implemented as a GL fragment shader.
// Requires OpenGL 3.0 or greater.

const float kPi = 3.14159;
const int kMaxNumWaves = 15;

// Parameters set outside of shader.
uniform float t;           // time
uniform vec2 resolution;   // screen resolution
uniform int num_waves;     // number of waves
uniform float freq;        // spatial frequency of waves

uniform sampler1D phases;

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
    vec4 phase_vec = texture1D(phases, (float(w) + 0.5) / float(kMaxNumWaves));
    float phase = t * phase_vec.r;
    p += 0.5 * (cos(freq * (cx + sy) + phase) + 1.0);
  }

  // General Rotors patented color mixer:
  float cc = cos(kPi * p);
  float ss = sin(kPi * p);
  const float ra = 0.0;
  const float ga = 0.2 * kPi;
  const float ba = 0.5 * kPi;
  const float k = 1.6;
  const float d = 0.7;
  float rr = cos(ra) * cc + sin(ra) * ss;
  float gg = cos(ga) * cc + sin(ga) * ss;
  float bb = cos(ba) * cc + sin(ba) * ss;
  float r = k * 0.5 * rr + d;
  float g = k * 0.5 * gg + d;
  float b = k * 0.5 * bb + d;

  gl_FragColor = vec4(r, g, b, 1.0);
}
