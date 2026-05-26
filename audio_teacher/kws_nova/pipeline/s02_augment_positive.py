import numpy as np
import os
import random
from scipy.io import wavfile

# --- 输入参数 ---
# 假设 PCM 文件为 16000 Hz 采样率, 16-bit 编码 (常用的 KWS 标准)
SAMPLE_RATE = 16000
SAMPLE_WIDTH = 2  # 16-bit = 2 bytes

# 输入目录和文件
NOISE_FILE = 'dataset/d0_pcm16/d00_background/background.pcm'
KEYWORD_DIR = 'dataset/d0_pcm16/d01_positive/'

# 输出目录
OUTPUT_DIR = 'dataset/d0_pcm16/d02_augment_positive/'

# --- 扩增配置 ---
# 目标信噪比 (SNR) 范围 (dB)。我们将在这个范围内随机选择 SNR 进行混合
SNR_RANGE_DB = [0, 5, 10, 15]  # 例如：从微弱噪音 (0dB) 到较弱噪音 (15dB)
# 每个原始样本的目标增强次数
AUGMENTATION_COUNT_PER_SAMPLE = 10 
# 100个原始样本 * 10次增强 = 1000个增强样本 + 100个原始样本 = 1100个最终样本

# --- 辅助函数：计算功率和调整增益 ---

def calculate_rms(audio_data):
    """计算音频数据的均方根值 (RMS), 代表信号强度/功率."""
    # 确保数据是浮点数以避免溢出
    audio_data = audio_data.astype(np.float64) 
    return np.sqrt(np.mean(audio_data**2))

def adjust_gain_for_snr(signal, noise_segment, snr_db):
    """
    调整噪音片段的增益，使其与信号片段达到指定的信噪比 (SNR)。
    
    公式: SNR_dB = 10 * log10(P_signal / P_noise)
    P_noise = P_signal / 10^(SNR_dB / 10)
    增益因子 G = sqrt(P_noise_target / P_noise_actual)
    """
    rms_signal = calculate_rms(signal)
    rms_noise = calculate_rms(noise_segment)
    
    if rms_noise == 0:
        return noise_segment # 避免除以零
    
    # 将 SNR 从 dB 转换为线性功率比
    linear_power_ratio = 10**(snr_db / 10.0)
    
    # 计算目标噪音功率的 RMS (P_noise_target = P_signal / linear_power_ratio)
    # RMS_noise_target = RMS_signal / sqrt(linear_power_ratio)
    target_rms_noise = rms_signal / np.sqrt(linear_power_ratio)
    
    # 计算增益因子
    gain_factor = target_rms_noise / rms_noise
    
    # 应用增益
    adjusted_noise = noise_segment * gain_factor
    return adjusted_noise.astype(signal.dtype)


# --- 主脚本 ---

def augment_data():
    """执行数据扩增和保存。"""
    
    # 1. 确保输出目录存在
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    
    # 2. 读取噪音数据
    print(f"读取背景噪音文件: {NOISE_FILE}...")
    try:
        # 使用 np.fromfile 读取 PCM 原始字节，指定数据类型为 16-bit 整数
        noise_raw = np.fromfile(NOISE_FILE, dtype=np.int16)
    except FileNotFoundError:
        print(f"错误: 找不到噪音文件 {NOISE_FILE}")
        return
        
    # 确定正样本的长度 (1秒 = 16000 个样本点)
    keyword_length = SAMPLE_RATE 
    
    # 3. 遍历并处理每个关键词样本
    
    # 获取关键词文件列表
    keyword_files = sorted([f for f in os.listdir(KEYWORD_DIR) if f.endswith('.pcm')])
    if not keyword_files:
        print(f"错误: 在目录 {KEYWORD_DIR} 中未找到任何 .pcm 文件。")
        return

    total_files_processed = 0
    
    for i, filename in enumerate(keyword_files):
        keyword_path = os.path.join(KEYWORD_DIR, filename)
        base_name = os.path.splitext(filename)[0]
        
        # 读取关键词数据
        try:
            keyword_data = np.fromfile(keyword_path, dtype=np.int16)
        except FileNotFoundError:
            print(f"警告: 找不到关键词文件 {keyword_path}，跳过。")
            continue
            
        if len(keyword_data) != keyword_length:
            print(f"警告: 文件 {filename} 长度不符合 1 秒 ({keyword_length} 样本点)，跳过。")
            continue

        # --- A. 保存原始样本 ---
        output_path_raw = os.path.join(OUTPUT_DIR, f'{base_name}_RAW.pcm')
        keyword_data.tofile(output_path_raw)
        total_files_processed += 1
        
        # --- B. 数据增强 ---
        for j in range(AUGMENTATION_COUNT_PER_SAMPLE):
            
            # 1. 随机选择一个 SNR 值
            snr_db = random.choice(SNR_RANGE_DB)
            
            # 2. 从噪音文件中随机选择一个 1秒 的片段
            max_start = len(noise_raw) - keyword_length
            if max_start < 0:
                 print("错误: 噪音文件太短，无法切出足够的 1 秒片段。")
                 return
                 
            start_index = random.randint(0, max_start)
            noise_segment = noise_raw[start_index : start_index + keyword_length]
            
            # 3. 调整噪音片段的增益以达到目标 SNR
            adjusted_noise = adjust_gain_for_snr(keyword_data, noise_segment, snr_db)
            
            # 4. 混合信号
            # 直接相加，但需要检查是否会裁剪 (clipping)
            mixed_data_float = keyword_data.astype(np.float64) + adjusted_noise.astype(np.float64)
            
            # 5. 限制混合后的信号范围到 16-bit 的边界 [-32768, 32767]
            mixed_data = np.clip(mixed_data_float, -32768, 32767).astype(np.int16)

            # 6. 保存增强后的样本
            output_path_aug = os.path.join(OUTPUT_DIR, f'{base_name}_AUG_{j:02d}_SNR{snr_db}.pcm')
            mixed_data.tofile(output_path_aug)
            total_files_processed += 1
            
        print(f"已处理 {filename}，生成 {AUGMENTATION_COUNT_PER_SAMPLE} 个增强样本。")

    print("--- 扩增完成 ---")
    print(f"总共生成或复制了 {total_files_processed} 个正样本文件到 {OUTPUT_DIR}")


if __name__ == '__main__':
    augment_data()