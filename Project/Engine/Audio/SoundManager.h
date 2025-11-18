#pragma once
#include <xaudio2.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mfobjects.h>
#include <mfmediaengine.h>
#include <Mmreg.h>

#include <fstream>
#include <wrl.h>
#include <unordered_map>
#include <memory>
#include <string>

#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

// チャンクヘッダ（WAV用）
struct ChunkHeader {
    char id[4]; // チャンクID
    int32_t size; // チャンクサイズ
};

// RIFFヘッダチャンク（WAV用）
struct RiffHeader {
    ChunkHeader chunk; // RIFF
    char type[4]; // WAVE
};

// FMTチャンク（WAV用）
struct FormatChunk {
    ChunkHeader chunk; // FMT
    WAVEFORMATEX fmt; // 波形フォーマット
};

// 音声データ
struct SoundData {
    // 波形フォーマット
    WAVEFORMATEX wfex;
    // バッファの先頭アドレス
    BYTE* pBuffer;
    // バッファのサイズ
    unsigned int bufferSize;
    // ファイル形式（wav, mp3など）
    std::string format;
    
    // 初期化
    SoundData() : pBuffer(nullptr), bufferSize(0), format("") {
        ZeroMemory(&wfex, sizeof(WAVEFORMATEX));
    }
    
    // デストラクタ
    ~SoundData() {
        if (pBuffer) {
            delete[] pBuffer;
            pBuffer = nullptr;
        }
    }
};

// サウンドハンドル（管理用）
using SoundHandle = size_t;

// ボイス管理クラス
class SoundVoice {
public:
    SoundVoice();
    ~SoundVoice();
    
    bool Initialize(IXAudio2* xAudio2, const SoundData& soundData);
    void Play(bool loop = false);
    void Stop();
    void Pause();
    void Resume();
    void SetVolume(float volume);
    float GetVolume() const;
    bool IsPlaying() const;
    bool IsPaused() const;
    
private:
    IXAudio2SourceVoice* sourceVoice_;
    bool isPlaying_;
    bool isPaused_;
    float volume_;
    XAUDIO2_BUFFER buffer_;
    
    void CleanupVoice();
};

class SoundManager {
public:
    SoundManager();
    ~SoundManager();
    
    // XAudio2とMedia Foundationの初期化
    bool Initialize();
    
    // 音声ファイルの読み込み（自動フォーマット判定）
    SoundHandle LoadSound(const std::string& filename);
    
    // WAV音声データの読み込み（従来の機能）
    SoundData LoadWaveFile(const std::string& filename);
    
    // MP3音声データの読み込み（新機能）
    SoundData LoadMP3File(const std::string& filename);
    
    // 音声データの解放
    void UnloadSound(SoundHandle handle);
    void UnloadSoundData(SoundData* soundData);
    
    // 音声再生（ハンドル使用）
    bool PlaySound(SoundHandle handle, bool loop = false);
    bool PlaySoundOneShot(SoundHandle handle);
    
    // 音声制御
    void StopSound(SoundHandle handle);
    void StopAllSounds();
    void PauseSound(SoundHandle handle);
    void ResumeSound(SoundHandle handle);
    
    // 音量制御
    void SetVolume(SoundHandle handle, float volume);
    float GetVolume(SoundHandle handle) const;
    void SetMasterVolume(float volume);
    float GetMasterVolume() const;
    
    // 状態取得
    bool IsPlaying(SoundHandle handle) const;
    bool IsPaused(SoundHandle handle) const;
    
    // xAudio2取得
    IXAudio2* GetXAudio2() { return xAudio2_.Get(); }
    
    // システム終了
    void Shutdown();
    
    // ===== 新しい便利メソッド =====
    /// @brief サウンドのロード、再生、管理を一行で行う便利メソッド
    /// @param filename ファイルパス
    /// @param loop ループ再生するか
    /// @param volume 音量 (0.0f ～ 1.0f)
    /// @return サウンドハンドル
    SoundHandle PlaySoundFile(const std::string& filename, bool loop = false, float volume = 1.0f);
    
    /// @brief サウンドの停止と解放を一行で行う便利メソッド
    /// @param handle サウンドハンドル
    void StopAndUnload(SoundHandle handle);
    
    /// @brief サウンドハンドルの自動管理クラス
    class SoundResource {
    public:
        SoundResource(SoundManager* manager, SoundHandle handle);
        ~SoundResource();
        
        // コピー禁止、ムーブ可能
        SoundResource(const SoundResource&) = delete;
        SoundResource& operator=(const SoundResource&) = delete;
        SoundResource(SoundResource&& other) noexcept;
        SoundResource& operator=(SoundResource&& other) noexcept;
        
        // サウンド操作
        bool Play(bool loop = false);
        void Stop();
        void Pause();
        void Resume();
        void SetVolume(float volume);
        float GetVolume() const;
        bool IsPlaying() const;
        bool IsPaused() const;
        
        // ハンドル取得
        SoundHandle GetHandle() const { return handle_; }
        
        // 有効性チェック
        bool IsValid() const { return handle_ != 0 && manager_ != nullptr; }
        
    private:
        SoundManager* manager_;
        SoundHandle handle_;
    };
    
    /// @brief サウンドリソースの作成（RAII管理）
    /// @param filename ファイルパス
    /// @return 自動管理されるサウンドリソース
    std::unique_ptr<SoundResource> CreateSoundResource(const std::string& filename);
    
    // ===== 後方互換性のための従来型メソッド =====
    // 従来の音声データの読み込み（非推奨だが互換性のため残す）
    SoundData SoundLoadWave(const char* filename);
    
    // 従来の音声データの解放（非推奨だが互換性のため残す）
    void SoundUnload(SoundData* soundData);
    
    // 従来の音声データの再生（非推奨だが互換性のため残す）
    void SoundPlayWave(const SoundData& soundData);
    
private:
    Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
    IXAudio2MasteringVoice* masteringVoice_;
    
    // Media Foundation関連
    bool mfInitialized_;
    
    // サウンドデータ管理
    std::unordered_map<SoundHandle, std::unique_ptr<SoundData>> soundDataMap_;
    std::unordered_map<SoundHandle, std::unique_ptr<SoundVoice>> soundVoiceMap_;
    SoundHandle nextHandle_;

    // まだボイスが作られていないサウンドの希望音量を保持
    std::unordered_map<SoundHandle, float> pendingVolume_;
    
    // マスター音量
    float masterVolume_;
    
    // ヘルパーメソッド
    bool InitializeMediaFoundation();
    void ShutdownMediaFoundation();
    std::string GetFileExtension(const std::string& filename) const;
    SoundHandle GenerateHandle();
    
    // Media FoundationでPCMデータを取得
    bool ExtractPCMDataFromFile(const std::string& filename, SoundData& outSoundData);
    
    // WAVファイル読み込み（従来の実装）
    bool LoadWaveFileInternal(const std::string& filename, SoundData& outSoundData);
};

// ===== 簡潔な型エイリアス（冗長性解決） =====
/// @brief 最も簡潔なサウンドリソース型（推奨）
using Sound = std::unique_ptr<SoundManager::SoundResource>;

/// @brief やや詳細なサウンドリソース型
using SoundPtr = std::unique_ptr<SoundManager::SoundResource>;

/// @brief 完全に明示的なサウンドリソース型
using SoundResourcePtr = std::unique_ptr<SoundManager::SoundResource>;
