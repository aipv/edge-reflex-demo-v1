import os
import asyncio
import subprocess
import random
import numpy as np
import soundfile as sf

import edge_tts

# ============================================================
# 输出:
#
# dataset/positive/
#    nova_0000_xxx.pcm
#
# PCM格式:
#    16kHz
#    mono
#    int16
# ============================================================

# =========================
# 配置
# =========================

OUTPUT_DIR = "dataset/d0_pcm16/d01_positive"

SAMPLE_RATE = 16000

CLIP_DURATION = 1.0

# 生成多少样本
TARGET_COUNT = 100

# TTS phrases
PHRASES = [
    "Nova",
    "Hey Nova",
    "Hello Nova",
    "OK Nova",
]

# Voices
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

os.makedirs(OUTPUT_DIR, exist_ok=True)

# =========================
# 工具函数
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

    await communicate.save(wav_file)

def wav_to_pcm(
    wav_file,
    pcm_file
):
    """
    ffmpeg:
        wav -> 16k mono s16le pcm
    """

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

def center_crop_pcm(
    pcm_file,
    output_file
):
    """
    裁成固定1秒PCM
    """

    audio = np.fromfile(
        pcm_file,
        dtype=np.int16
    )

    target_samples = int(
        SAMPLE_RATE * CLIP_DURATION
    )

    # 如果太短
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

    # 长度不足再pad
    if len(clip) < target_samples:

        padded = np.zeros(
            target_samples,
            dtype=np.int16
        )

        padded[:len(clip)] = clip

        clip = padded

    clip.tofile(output_file)

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
        # 最终输出
        # =====================

        output_file = os.path.join(
            OUTPUT_DIR,
            f"nova_{i:04d}.pcm"
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