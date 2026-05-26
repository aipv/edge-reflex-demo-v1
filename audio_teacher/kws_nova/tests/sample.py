import os
import sys
import random
import numpy as np

from faster_whisper import WhisperModel
from rapidfuzz import fuzz

# ============================================================
# 用法:
#
# python extract_nova.py input.pcm
#
# 输出:
#
# dataset/
#    positive/
#       nova_0000_1734.pcm
#
#    negative/
#       negative_0000_5321.pcm
#
# PCM格式:
#    16kHz
#    mono
#    int16
#    little-endian
# ============================================================

# =========================
# 参数检查
# =========================

if len(sys.argv) < 2:
    print("Usage:")
    print("python extract_nova.py input.pcm")
    sys.exit(1)

PCM_FILE = sys.argv[1]

if not os.path.exists(PCM_FILE):
    print(f"File not found: {PCM_FILE}")
    sys.exit(1)

# =========================
# 配置
# =========================

SAMPLE_RATE = 16000

KEYWORD = "nova"

FUZZ_THRESHOLD = 80

# clip长度（秒）
CLIP_DURATION = 1.0

MODEL_SIZE = "small.en"

# 每个 positive 生成多少 negative
NEGATIVE_PER_POSITIVE = 5

# negative 避开 positive 的 buffer（秒）
NEGATIVE_BUFFER = 0.5

# 输出目录
POSITIVE_DIR = "dataset/positive"
NEGATIVE_DIR = "dataset/negative"

# =========================
# 创建目录
# =========================

os.makedirs(POSITIVE_DIR, exist_ok=True)
os.makedirs(NEGATIVE_DIR, exist_ok=True)

# =========================
# 初始化 Whisper
# =========================

print("Loading Whisper model...")

model = WhisperModel(
    MODEL_SIZE,
    device="cpu",
    compute_type="int8"
)

# =========================
# 读取 PCM
# =========================

print(f"Loading PCM: {PCM_FILE}")

audio_int16 = np.fromfile(
    PCM_FILE,
    dtype=np.int16
)

audio_float32 = (
    audio_int16.astype(np.float32) / 32768.0
)

audio_duration = (
    len(audio_int16) / SAMPLE_RATE
)

print(f"Audio duration: {audio_duration:.2f}s")

# =========================
# Whisper 转写
# =========================

print("Running transcription...")

segments, info = model.transcribe(
    audio_float32,
    language="en",
    word_timestamps=True
)

# =========================
# Positive sample generation
# =========================

positive_zones = []

positive_index = 0

print("\nSearching keyword...")

for segment in segments:

    print("\nSEGMENT:")
    print(segment.text)

    if segment.words is None:
        continue

    for word in segment.words:

        text = word.word.strip().lower()

        start_time = float(word.start)
        end_time = float(word.end)

        print(
            f"WORD={text} "
            f"START={start_time:.2f} "
            f"END={end_time:.2f}"
        )

        score = fuzz.ratio(text, KEYWORD)

        if score >= FUZZ_THRESHOLD:

            print("\nFOUND NOVA!")

            print(
                f"timestamp: "
                f"{start_time:.2f} -> {end_time:.2f}"
            )

            # ====================================
            # 固定长度 positive clip
            # ====================================

            center = (
                start_time + end_time
            ) / 2.0

            clip_start = max(
                0,
                center - CLIP_DURATION / 2
            )

            clip_end = (
                clip_start + CLIP_DURATION
            )

            # 防止越界
            if clip_end > audio_duration:
                clip_end = audio_duration
                clip_start = (
                    clip_end - CLIP_DURATION
                )

            print(
                f"positive clip: "
                f"{clip_start:.2f} -> "
                f"{clip_end:.2f}"
            )

            # 保存 positive zone
            positive_zones.append(
                (
                    clip_start,
                    clip_end
                )
            )

            # 时间 -> sample
            start_sample = int(
                clip_start * SAMPLE_RATE
            )

            end_sample = int(
                clip_end * SAMPLE_RATE
            )

            # 裁剪
            clip = audio_int16[
                start_sample:end_sample
            ]

            # 文件名时间(ms)
            time_ms = int(
                clip_start * 1000
            )

            output_file = os.path.join(
                POSITIVE_DIR,
                f"nova_{positive_index:04d}_{time_ms}.pcm"
            )

            # 保存
            clip.tofile(output_file)

            print(
                f"saved positive: {output_file}"
            )

            positive_index += 1

print(
    f"\nGenerated "
    f"{positive_index} positive samples"
)

# =========================
# Negative sample generation
# =========================

print("\nGenerating negative samples...")

negative_index = 0

target_negative_count = (
    positive_index * NEGATIVE_PER_POSITIVE
)

negative_zones = []

def overlap(a1, a2, b1, b2):
    return max(a1, b1) < min(a2, b2)

attempt = 0
max_attempt = 10000

while (
    negative_index < target_negative_count
    and attempt < max_attempt
):

    attempt += 1

    # ====================================
    # 随机时间
    # ====================================

    clip_start = random.uniform(
        0,
        audio_duration - CLIP_DURATION
    )

    clip_end = (
        clip_start + CLIP_DURATION
    )

    bad = False

    # ====================================
    # 避开 positive zones
    # ====================================

    for p_start, p_end in positive_zones:

        if overlap(
            clip_start,
            clip_end,
            p_start - NEGATIVE_BUFFER,
            p_end + NEGATIVE_BUFFER
        ):
            bad = True
            break

    if bad:
        continue

    # ====================================
    # 避开已有 negative
    # 防止 negative 大量重叠
    # ====================================

    for n_start, n_end in negative_zones:

        if overlap(
            clip_start,
            clip_end,
            n_start,
            n_end
        ):
            bad = True
            break

    if bad:
        continue

    # ====================================
    # 时间 -> sample
    # ====================================

    start_sample = int(
        clip_start * SAMPLE_RATE
    )

    end_sample = int(
        clip_end * SAMPLE_RATE
    )

    # ====================================
    # 裁剪
    # ====================================

    clip = audio_int16[
        start_sample:end_sample
    ]

    # ====================================
    # 保存 negative
    # ====================================

    time_ms = int(
        clip_start * 1000
    )

    output_file = os.path.join(
        NEGATIVE_DIR,
        f"negative_{negative_index:04d}_{time_ms}.pcm"
    )

    clip.tofile(output_file)

    # 保存 negative zone
    negative_zones.append(
        (
            clip_start,
            clip_end
        )
    )

    print(
        f"saved negative: {output_file}"
    )

    negative_index += 1

# =========================
# 完成
# =========================

print("\nDone.")

print(
    f"Generated "
    f"{positive_index} positive samples"
)

print(
    f"Generated "
    f"{negative_index} negative samples"
)