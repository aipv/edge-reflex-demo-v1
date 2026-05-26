import os
import asyncio
import subprocess
import random
import numpy as np

import edge_tts

# ============================================================
# 输出:
#
# dataset/normal_negative/
#    normal_negative_0000_xxx.pcm
#
# PCM格式:
#    16kHz
#    mono
#    int16
# ============================================================

# =========================
# 配置
# =========================

OUTPUT_DIR = "dataset/d0_pcm16/d04_normal_negative"

SAMPLE_RATE = 16000

CLIP_DURATION = 1.0

TARGET_COUNT = 2000

# ====================================
# 普通语音句子
# 不包含 Nova
# ====================================

PHRASES = [

    "How are you today",
    "Good morning",
    "Turn on the light",
    "I am going to work",
    "Can you help me",
    "What time is it",
    "Please open the door",
    "The weather is very nice",
    "I would like some coffee",
    "Could you repeat that",
    "This is a simple sentence",
    "The meeting starts at noon",
    "Please call me later",
    "I need more information",
    "The music is too loud",
    "We are going to the office",
    "I will be back tomorrow",
    "Thank you very much",
    "This is only a test",
    "The computer is running",
    "Please turn off the TV",
    "I want to buy a new phone",
    "Can you hear me clearly",
    "The train arrives at eight",
    "I forgot my password",
    "The lights are very bright",
    "This room is too cold",
    "I need to charge my laptop",
    "Please send me the file",
    "The internet connection is slow",

    # 更长句子
    "I am going to the office tomorrow morning",
    "Can you please help me with this project",
    "The weather today is much better than yesterday",
    "I would like to order a cup of coffee",
    "Please remember to close the window before leaving",
    "The train station is very crowded this evening",
    "I think we should leave a little earlier today",
    "Could you please repeat the last sentence again",
    "The restaurant near the office is very popular",
    "I need to finish this work before dinner",

]

# voices
VOICES = [
    "en-US-GuyNeural",
    "en-US-JennyNeural",
    "en-GB-RyanNeural",
    "en-GB-SoniaNeural",
]

# rate
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
# 随机裁剪
# =========================

def random_crop_pcm(
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

    # 太短则padding
    if len(audio) < target_samples:

        padded = np.zeros(
            target_samples,
            dtype=np.int16
        )

        padded[:len(audio)] = audio

        audio = padded

    # 随机crop
    max_start = max(
        0,
        len(audio) - target_samples
    )

    start = random.randint(
        0,
        max_start
    )

    end = start + target_samples

    clip = audio[start:end]

    # 再次padding
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
            f"normal_negative_{i:04d}.pcm"
        )

        random_crop_pcm(
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