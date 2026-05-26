import os
import glob
import numpy as np
from scipy.fft import fft as scipy_fft # 使用scipy的fft函数
from scipy.fft import dct as scipy_dct # 使用scipy的dct函数

FS = 16000
N_FFT = 512
N_MFCC = 13
N_MEL_FILTERS = 40
N_FRAME = 98
HOP_SIZE = 160
FRAME_SIZE = 400

def _hz_to_mel(hz):
    """赫兹 (Hz) 转换为 Mel 刻度"""
    return 2595.0 * np.log10(1.0 + hz / 700.0)

def _mel_to_hz(mel):
    """Mel 刻度转换为赫兹 (Hz)"""
    return 700.0 * (10.0**(mel / 2595.0) - 1.0)

def _get_mel_filter_bank(fs, n_fft, n_mel_filters):
    """
    生成 Mel 滤波器组矩阵
    返回形状: (n_mel_filters, n_fft/2 + 1)
    """
    # 频率分辨率
    # bin_width = fs / n_fft
    
    # 1. 在 Mel 刻度上均匀分布中心频率
    low_mel = _hz_to_mel(0)
    high_mel = _hz_to_mel(fs / 2)
    # 在 Mel 轴上生成 n_mel_filters + 2 个点（包括低、高边界和 n_mel_filters 个中心点）
    mel_points = np.linspace(low_mel, high_mel, n_mel_filters + 2)
    
    # 2. 将 Mel 刻度点转换回 Hz，并找到对应的 FFT Bin 索引
    hz_points = _mel_to_hz(mel_points)
    # 将 Hz 频率转换为最近的 FFT bin 索引
    bin_points = np.floor((n_fft + 1) * hz_points / fs).astype(int)
    
    # 3. 构建 Mel 滤波器矩阵
    filter_bank = np.zeros((n_mel_filters, n_fft // 2 + 1))
    
    for m in range(1, n_mel_filters + 1):
        f_m_minus = bin_points[m - 1] # 左边界
        f_m = bin_points[m]           # 中心点
        f_m_plus = bin_points[m + 1]  # 右边界
        
        # 三角形滤波器的上升部分 (从左边界到中心点)
        for k in range(f_m_minus, f_m):
            if k >= 0 and k < filter_bank.shape[1]:
                # 线性斜坡
                filter_bank[m - 1, k] = (k - f_m_minus) / (f_m - f_m_minus)
        
        # 三角形滤波器的下降部分 (从中心点到右边界)
        for k in range(f_m, f_m_plus):
            if k >= 0 and k < filter_bank.shape[1]:
                # 线性斜坡
                filter_bank[m - 1, k] = (f_m_plus - k) / (f_m_plus - f_m)
                
    return filter_bank


def mfcc_feature_one_frame(input_samples: np.ndarray) -> np.ndarray:
    assert len(input_samples) == 400, "输入必须是400个采样点"
    padded_array = np.pad(input_samples, pad_width=(0, 112), mode='constant', constant_values=0)
    float_samples = padded_array.astype(np.float32)
    window = np.hanning(N_FFT)
    windowed_samples = float_samples * window
    fft_result = scipy_fft(windowed_samples, N_FFT)
    fft_magnitude = np.abs(fft_result[:N_FFT // 2 + 1])
    power_spectrum = (fft_magnitude ** 2) / N_FFT
    mel_basis = _get_mel_filter_bank(FS, N_FFT, N_MEL_FILTERS)
    mel_energies = np.dot(power_spectrum, mel_basis.T)
    log_mel_energies = np.log(mel_energies.clip(min=1e-12))
    mfccs = scipy_dct(log_mel_energies, type=2, axis=-1, norm='ortho')
    final_mfccs = mfccs[:N_MFCC]
    return final_mfccs


def mfcc_feature_one_file(filepath):
    pcm = np.frombuffer(open(filepath, "rb").read(), dtype=np.int16)
    result = np.zeros((N_FRAME, N_MFCC))
    for i in range(N_FRAME):
        start = i * HOP_SIZE;
        end = i * HOP_SIZE + FRAME_SIZE
        result[i] = mfcc_feature_one_frame(pcm[start:end])
    return result

def get_input_file(input_fold):
    input_files = glob.glob(os.path.join(input_fold, '*.pcm'))
    if not input_files:
        print(f"错误: 在目录 {input_fold} 中未找到任何 .bin 文件。请检查路径。")
        return
    print(f"总共找到 {len(input_files)} 个扩增音频文件。开始批量提取 MFCC...")
    return input_files

def mfcc_batch_process(input_fold, output_fold):
    processed_count = 0
    input_files = get_input_file(input_fold)
    for file_path in input_files:
        filename = os.path.basename(file_path)
        base_name = os.path.splitext(filename)[0]
        mfcc_features = mfcc_feature_one_file(file_path)
        if mfcc_features is None:
            print("\n--- MFCC return None! ---")
            return
        output_file_path = os.path.join(output_fold, f"{base_name}.npy")
        np.save(output_file_path, mfcc_features)
        processed_count += 1
        if processed_count % 100 == 0:
            print(f"已处理 {processed_count} 个文件...")

    print(f"\n========================================================")
    print(f"✅ 批量 MFCC 提取完成！总共处理了 {processed_count} 个文件。")
    print(f"MFCC 特征已保存到目录: {output_fold}")
    print(f"========================================================")
