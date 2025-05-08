
#ifndef CONTROLS_H
#define CONTROLS_H

inline constexpr float SIMULATION_SPEED_DEFAULT = 100.0f;
inline constexpr char SIMULATION_SPEED_KEY = '1';

inline constexpr float GRAVITY_X_DEFAULT = 0.0f;
inline constexpr char GRAVITY_X_KEY = '2';

inline constexpr float GRAVITY_Y_DEFAULT = 0.4f;
inline constexpr char GRAVITY_Y_KEY = '3';

inline constexpr float EDGE_BOUNCINESS_DEFAULT = 0.0f;
inline constexpr char EDGE_BOUNCINESS_KEY = '4';

inline constexpr float INTERACTION_RADIUS_DEFAULT = 60.0f;
inline constexpr char INTERACTION_RADIUS_KEY = '5';

inline constexpr float REST_DENSITY_DEFAULT = 6.0f;
inline constexpr char REST_DENSITY_KEY = '6';

inline constexpr float STIFFNESS_DEFAULT = 0.5f;
inline constexpr char STIFFNESS_KEY = '7';

inline constexpr float NEAR_STIFFNESS_DEFAULT = 0.5f;
inline constexpr char NEAR_STIFFNESS_KEY = '8';

inline constexpr float LINEAR_VISCOSITY_DEFAULT = 0.0f;
inline constexpr char LINEAR_VISCOSITY_KEY = '9';

inline constexpr float QUADRATIC_VISCOSITY_DEFAULT = 0.0f;
inline constexpr char QUADRATIC_VISCOSITY_KEY = '0';

inline constexpr float PLASTICITY_DEFAULT = 0.0f;
inline constexpr char PLASTICITY_KEY = 'Q';

inline constexpr float YIELD_RATIO_DEFAULT = 0.2f;
inline constexpr char YIELD_RATIO_KEY = 'W';

inline constexpr float SPRING_STIFFNESS_DEFAULT = 0.5f;
inline constexpr char SPRING_STIFFNESS_KEY = 'E';

inline constexpr float CONTROL_RADIUS_DEFAULT = 50.0f;
inline constexpr char CONTROL_RADIUS_KEY = 'R';

inline constexpr float PARTICLE_SPAWN_RATE_DEFAULT = 3.0f;
inline constexpr char PARTICLE_SPAWN_RATE_KEY = 'T';

inline constexpr float BASE_PARTICLE_SIZE_DEFAULT = 5.0f;
inline constexpr char BASE_PARTICLE_SIZE_KEY = 'Y';

inline constexpr float PARTICLE_STRESS_SIZE_MULTIPLIER_DEFAULT = 5.0f;
inline constexpr char PARTICLE_STRESS_SIZE_MULTIPLIER_KEY = 'U';

inline constexpr float BASE_PARTICLE_COLOR_DEFAULT = 220.0f;
inline constexpr char BASE_PARTICLE_COLOR_KEY = 'I';

inline constexpr float PARTICLE_STRESS_COLOR_MULTIPLIER_DEFAULT = 100.0f;
inline constexpr char PARTICLE_STRESS_COLOR_MULTIPLIER_KEY = 'O';

#endif
