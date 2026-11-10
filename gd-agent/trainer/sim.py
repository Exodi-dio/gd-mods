"""Minimal Geometry Dash cube physics sim for training."""
import numpy as np
import gymnasium as gym
from gymnasium import spaces

# Physics constants (tuned to approximate GD cube)
GRAVITY = -2000.0          # px/s^2 (downward)
JUMP_VEL = 700.0           # initial jump velocity
JUMP_HOLD_EXTRA = 150.0    # extra velocity while holding (short hop if released)
SPEED = 300.0              # horizontal speed px/s
GROUND_Y = 100.0           # ground level
CEILING_Y = 400.0          # ceiling (unused for cube, but for completeness)
DT = 1.0 / 60.0            # 60 fps

class GDLevel:
    """Generates a level as a list of (type, x, y, w, h). type: 0=spike, 1=block."""
    def __init__(self, length=2000.0, seed=None):
        self.rng = np.random.default_rng(seed)
        self.length = length
        self.obstacles = self._gen()

    def _gen(self):
        obs = []
        x = 400.0
        while x < self.length - 200:
            if self.rng.random() < 0.6:
                # spike
                obs.append((0, x, GROUND_Y, 30, 30))
                x += self.rng.uniform(120, 220)
            else:
                # block (jump over or land on)
                h = self.rng.choice([30, 60])
                obs.append((1, x, GROUND_Y, 40, h))
                x += self.rng.uniform(150, 260)
        return obs

class GDEnv(gym.Env):
    metadata = {"render_modes": []}

    def __init__(self, level_seed=None):
        super().__init__()
        self.observation_space = spaces.Box(low=-np.inf, high=np.inf, shape=(8,), dtype=np.float32)
        self.action_space = spaces.Discrete(2)  # 0 release, 1 tap/hold
        self.level = GDLevel(seed=level_seed)
        self.reset()

    def _nearest_obstacle(self):
        px = self.px
        best = None
        best_dist = 1e9
        for (t, ox, oy, ow, oh) in self.level.obstacles:
            if ox + ow/2 > px and (ox - px) < best_dist:
                best_dist = ox - px
                best = (t, ox, oy, ow, oh)
        if best is None:
            return 1.0, 0.0, 0.0, 0.0  # no obstacle ahead
        t, ox, oy, ow, oh = best
        return (ox - px) / 400.0, oy / 320.0, oh / 100.0, t / 1.0

    def _obs(self):
        on_ground = 1.0 if self.on_ground else 0.0
        next_x, next_y, next_h, next_type = self._nearest_obstacle()
        return np.array([
            self.py / 320.0,
            self.vy / 100.0,
            on_ground,
            next_x,
            next_y,
            next_h,
            next_type,
            1.0,  # speed (constant for now)
        ], dtype=np.float32)

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        if seed is not None:
            self.level = GDLevel(seed=seed)
        self.px = 0.0
        self.py = GROUND_Y + 15  # cube center
        self.vy = 0.0
        self.on_ground = True
        self.holding = False
        self.alive = True
        self.progress = 0.0
        self.steps = 0
        return self._obs(), {}

    def step(self, action):
        if not self.alive:
            return self._obs(), 0.0, True, False, {}

        self.holding = (action == 1)

        # physics
        if self.on_ground and self.holding:
            self.vy = JUMP_VEL
            self.on_ground = False
        # apply gravity
        self.vy += GRAVITY * DT
        if self.holding and self.vy > 0:
            self.vy += JUMP_HOLD_EXTRA * DT  # hold extends jump
        self.py += self.vy * DT
        self.px += SPEED * DT

        # ground collision
        if self.py <= GROUND_Y + 15:
            self.py = GROUND_Y + 15
            self.vy = 0.0
            self.on_ground = True

        # obstacle collision
        reward = 0.1  # survival reward
        done = False
        for (t, ox, oy, ow, oh) in self.level.obstacles:
            # simple AABB between cube (30x30 centered at px,py) and obstacle
            cx, cy, cw, ch = self.px, self.py, 30, 30
            ox1, oy1 = ox, oy
            ox2, oy2 = ox + ow, oy + oh
            cx1, cy1 = cx - cw/2, cy - ch/2
            cx2, cy2 = cx + cw/2, cy + ch/2
            if cx2 > ox1 and cx1 < ox2 and cy2 > oy1 and cy1 < oy2:
                # collision
                if t == 0:  # spike -> death
                    self.alive = False
                    done = True
                    reward = -10.0
                    break
                else:  # block -> land on top if falling
                    if self.vy < 0 and cy1 >= oy2 - 5:
                        self.py = oy2 + ch/2
                        self.vy = 0.0
                        self.on_ground = True
                    else:
                        self.alive = False
                        done = True
                        reward = -10.0
                        break

        self.progress = self.px / self.level.length
        if self.progress >= 1.0:
            done = True
            reward += 100.0
            self.alive = False

        self.steps += 1
        if self.steps > 10000:
            done = True

        return self._obs(), reward, done, False, {"progress": self.progress}
EOF
echo "written"