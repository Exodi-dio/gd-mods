"""PPO trainer for GD cube agent. Run on Kaggle/Colab (free GPU)."""
import os, struct, numpy as np
from stable_baselines3 import PPO
from stable_baselines3.common.env_util import make_vec_env
from stable_baselines3.common.callbacks import EvalCallback, CheckpointCallback
from sim import GDEnv

OBS_DIM, HIDDEN, ACTION_DIM = 8, 32, 1

def train(total_timesteps=2_000_000, n_envs=8, seed=0):
    env = make_vec_env(lambda: GDEnv(), n_envs=n_envs, seed=seed)
    eval_env = make_vec_env(lambda: GDEnv(), n_envs=1, seed=seed+1)
    model = PPO("MlpPolicy", env, verbose=1, device="auto",
                policy_kwargs=dict(net_arch=[HIDDEN, HIDDEN]),
                learning_rate=3e-4, n_steps=2048, batch_size=64,
                n_epochs=10, gamma=0.99, gae_lambda=0.95)
    cb = [EvalCallback(eval_env, best_model_save_path="./best", log_path="./logs", eval_freq=10000),
          CheckpointCallback(save_freq=50000, save_path="./ckpt")]
    model.learn(total_timesteps=total_timesteps, callback=cb)
    model.save("gd_agent_ppo")
    return model

def export_weights(model, out_path="agent_weights.bin"):
    # Extract policy network weights. SB3 MlpPolicy: policy.mlp_extractor.policy_net (Sequential),
    # policy.action_net (Linear). We need 2 hidden layers + output.
    import torch
    pol = model.policy
    # policy_net is Sequential of Linear layers
    layers = list(pol.mlp_extractor.policy_net.children())
    # layers: [Linear(OBS->H), ReLU, Linear(H->H), ReLU]
    W1 = layers[0].weight.detach().cpu().numpy().astype("float32")  # H x OBS
    b1 = layers[0].bias.detach().cpu().numpy().astype("float32")
    W2 = layers[2].weight.detach().cpu().numpy().astype("float32")  # H x H
    b2 = layers[2].bias.detach().cpu().numpy().astype("float32")
    # action net
    an = pol.action_net
    W3 = an.weight.detach().cpu().numpy().astype("float32")  # ACTION x H
    b3 = an.bias.detach().cpu().numpy().astype("float32")

    with open(out_path, "wb") as f:
        f.write(W1.flatten().tobytes())
        f.write(b1.tobytes())
        f.write(W2.flatten().tobytes())
        f.write(b2.tobytes())
        f.write(W3.flatten().tobytes())
        f.write(b3.tobytes())
    print("exported", out_path, os.path.getsize(out_path))

if __name__ == "__main__":
    m = train()
    export_weights(m)
EOF
echo "written"