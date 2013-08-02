// Quasicrystals, implemented as a GL fragment shader.
// Requires OpenGL 3.0 or greater.

const float kPi = 3.14159;
const int kMaxNumWaves = 15;

// Parameters set outside of shader.
uniform float t;             // time
uniform vec2 resolution;     // screen resolution
uniform int num_waves;       // number of waves
uniform float spatial_freq;  // spatial frequency of waves
uniform float mix;           // mixing parameter for changing num_waves

uniform sampler1D phases;

void main() {
  float x = gl_FragCoord.x - 0.5 * resolution.x;
  float y = gl_FragCoord.y - 0.5 * resolution.y;

  // Weights for mixing in a new wave.
  float weights[kMaxNumWaves];
  for (int i = 0; i < kMaxNumWaves; ++i) {
    weights[i] = 1.0;
  }
  weights[1] = mix;

  // Initialize angles sines and coses.
  float coses[kMaxNumWaves]; 
  float sines[kMaxNumWaves];
  coses[0] = 1.0;
  sines[0] = 0.0;
  for (int i = 1; i < num_waves + 1; ++i) {
    float angle = (1.0 - mix) * float(i - 1) * kPi / float(num_waves) +
        mix * float(i) * kPi / float(num_waves + 1);
    coses[i] = cos(angle);
    sines[i] = sin(angle);
  }

  // Mixing together phases.
  float mixed_phases[kMaxNumWaves];
  vec4 phase_vec = texture1D(phases, (float(0.0) + 0.5) / float(kMaxNumWaves));
  mixed_phases[0] = t * phase_vec.r;
  for (int w = 1; w < num_waves + 1; ++w) {
    vec4 phase_vec0 = texture1D(phases, (float(w - 1) + 0.5) / float(kMaxNumWaves));
    vec4 phase_vec1 = texture1D(phases, (float(w) + 0.5) / float(kMaxNumWaves));
    mixed_phases[w] = t * ((1.0 - mix) * phase_vec0.r + mix * phase_vec1.r);
  }
  
  // Compute intensity over the sum of waves.
  float p = 0.0;
  for (int w = 0; w < num_waves + 1; ++w) {
    float cx = coses[w] * x;
    float sy = sines[w] * y;
    p += weights[w] * 0.5 * (cos(spatial_freq * (cx + sy) + mixed_phases[w]) + 1.0);
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
