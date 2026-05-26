import numpy as np

SAMPLE_RATE = 16000

# Whisper timestamp
start_time = 1.7
end_time = 2.1

# 输出 clip 长度（秒）
clip_duration = 1.0

# keyword center
center = (start_time + end_time) / 2

# clip 时间范围
clip_start = max(0, center - clip_duration / 2)
clip_end = clip_start + clip_duration

print("clip:", clip_start, clip_end)

# 读取 PCM
audio = np.fromfile(
    "test.pcm",
    dtype=np.int16
)

# 时间 -> sample index
start_sample = int(clip_start * SAMPLE_RATE)
end_sample = int(clip_end * SAMPLE_RATE)

# 裁剪
clip = audio[start_sample:end_sample]

# 保存 PCM
clip.tofile("nova_001.pcm")

print("saved")