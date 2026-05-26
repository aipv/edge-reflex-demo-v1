import os
import asyncio
import subprocess
import random
import numpy as np

import edge_tts

# ============================================================
# 输出:
#
# dataset/hard_negative/
#    hard_negative_0000_xxx.pcm
#
# PCM格式:
#    16kHz
#    mono
#    int16
# ============================================================

# =========================
# 配置
# =========================

OUTPUT_DIR = "dataset/d0_pcm16/d03_hard_negative"

SAMPLE_RATE = 16000

CLIP_DURATION = 1.0

TARGET_COUNT = 500

# ====================================
# 困难负样本（非常重要）
# ====================================

PHRASES = [

    # 类似 Nova
    "Nora",
    "Noah",
    "Nova?",
    "No way",
    "Nobody",
    "Never",
    "Nevada",
    "November",

    # 句子形式
    "Hello Nora",
    "Hey Noah",
    "No way home",
    "Never mind",
    "Nobody knows",
    "November rain",

    # 类似音节
    "Over",
    "Lower",
    "Lover",
    "Know her",
    "No more",

    # 更长句子
    "Hey Noah how are you",
    "Nobody is home",
    "Never do that again",
]

# voices
VOICES = [
    "en-US-GuyNeural",
    "en-US-JennyNeural",
    "en-GB-RyanNeural",
    "en-GB-SoniaNeural",
]

# 语速
RATES = [
    "-20%",
    "-10%",
    "+0%",
    "+10%",
]

# =========================
# 创建目录
# =========================

os.makedirs(
    OUTPUT_DIR,
    exist_ok=True
)

# =========================
# TTS
# =========================

async def generate_tts(
    text,
    voice,
    rate,
    wav_file
):

    communicate = edge_tts.Communicate(
        text=text,
        voice=voice,
        rate=rate
    )

    await communicate.save(
        wav_file
    )

# =========================
# WAV -> PCM
# =========================

def wav_to_pcm(
    wav_file,
    pcm_file
):

    cmd = [
        "ffmpeg",
        "-y",
        "-i",
        wav_file,

        "-f",
        "s16le",

        "-acodec",
        "pcm_s16le",

        "-ar",
        "16000",

        "-ac",
        "1",

        pcm_file
    ]

    subprocess.run(
        cmd,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL
    )

# =========================
# 中心裁剪
# =========================

def center_crop_pcm(
    pcm_file,
    output_file
):

    audio = np.fromfile(
        pcm_file,
        dtype=np.int16
    )

    target_samples = int(
        SAMPLE_RATE * CLIP_DURATION
    )

    # 太短则 padding
    if len(audio) < target_samples:

        padded = np.zeros(
            target_samples,
            dtype=np.int16
        )

        padded[:len(audio)] = audio

        audio = padded

    # 居中裁剪
    center = len(audio) // 2

    start = max(
        0,
        center - target_samples // 2
    )

    end = start + target_samples

    clip = audio[start:end]

    # 再次 padding
    if len(clip) < target_samples:

        padded = np.zeros(
            target_samples,
            dtype=np.int16
        )

        padded[:len(clip)] = clip

        clip = padded

    clip.tofile(
        output_file
    )

# =========================
# 主逻辑
# =========================

async def main():

    for i in range(TARGET_COUNT):

        phrase = random.choice(
            PHRASES
        )

        voice = random.choice(
            VOICES
        )

        rate = random.choice(
            RATES
        )

        print(
            f"[{i}] "
            f"{phrase} | "
            f"{voice} | "
            f"{rate}"
        )

        temp_wav = f"temp_{i}.wav"

        temp_pcm = f"temp_{i}.pcm"

        # =====================
        # TTS
        # =====================

        await generate_tts(
            phrase,
            voice,
            rate,
            temp_wav
        )

        # =====================
        # WAV -> PCM
        # =====================

        wav_to_pcm(
            temp_wav,
            temp_pcm
        )

        # =====================
        # 输出
        # =====================

        output_file = os.path.join(
            OUTPUT_DIR,
            f"hard_negative_{i:04d}.pcm"
        )

        center_crop_pcm(
            temp_pcm,
            output_file
        )

        print(
            f"saved: {output_file}"
        )

        # =====================
        # 清理
        # =====================

        if os.path.exists(temp_wav):
            os.remove(temp_wav)

        if os.path.exists(temp_pcm):
            os.remove(temp_pcm)

# =========================
# 启动
# =========================

asyncio.run(main())