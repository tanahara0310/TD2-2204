#pragma once
#include "IInputDevice.h"

#include <dinput.h>
#include <memory>
#include <vector>


class KeyboardInput;
class MouseInput;
class GamepadInput;

/// @brief 入力管理クラス
class InputManager {
public: // メンバ関数
    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize(HINSTANCE hInstance, HWND hwnd);

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// @brief キーボード入力を取得する関数
    /// @return キーボード入力オブジェクトへのポインタ
    KeyboardInput* GetKeyboard();

    /// @brief マウス入力を取得する関数
    /// @return マウス入力オブジェクトへのポインタ
    MouseInput* GetMouse();

    /// @brief ゲームパッド入力を取得する関数
    /// @return ゲームパッド入力オブジェクトへのポインタ
    GamepadInput* GetGamepad();

    /// @brief DirectInputオブジェクトを取得する関数
    /// @return DirectInputオブジェクトへのポインタ
    IDirectInput8* GetDirectInput() { return directInput_; }

private:
    // DirectInputオブジェクト
    IDirectInput8* directInput_ = nullptr;

    // デバイス関連
    std::vector<std::unique_ptr<IInputDevice>> devices_;
};
