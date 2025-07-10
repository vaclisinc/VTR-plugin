"""
VTR Model Export Script
將訓練好的PyTorch模型導出為C++可用的格式
"""

import torch
import json
import numpy as np
from sklearn.preprocessing import StandardScaler

def export_vtr_model(model, scaler, output_dir="./exported_model/"):
    """
    導出VTR模型參數為C++可用格式
    
    Args:
        model: 訓練好的PyTorch模型
        scaler: 訓練時使用的StandardScaler
        output_dir: 輸出目錄
    """
    import os
    os.makedirs(output_dir, exist_ok=True)
    
    # 1. 導出StandardScaler參數
    scaler_params = {
        "mean": scaler.mean_.tolist(),
        "std": scaler.scale_.tolist(),  # sklearn中scale_是標準差
        "feature_count": len(scaler.mean_)
    }
    
    with open(f"{output_dir}/scaler_params.json", "w") as f:
        json.dump(scaler_params, f, indent=2)
    
    # 2. 導出神經網絡權重
    model.eval()
    model_params = {}
    
    # 獲取每層的權重和偏置
    layers = list(model.net.children())
    layer_idx = 0
    
    for i, layer in enumerate(layers):
        if isinstance(layer, torch.nn.Linear):
            # 權重矩陣 (轉置以匹配C++的矩陣乘法順序)
            weight = layer.weight.detach().numpy()
            bias = layer.bias.detach().numpy()
            
            model_params[f"layer_{layer_idx}"] = {
                "weight": weight.tolist(),  # [output_dim, input_dim]
                "bias": bias.tolist(),
                "input_dim": weight.shape[1],
                "output_dim": weight.shape[0]
            }
            layer_idx += 1
    
    # 3. 導出模型元數據
    model_metadata = {
        "architecture": "MLP",
        "input_dim": 17,
        "hidden_dim": 64,
        "output_dim": 5,
        "num_layers": 3,
        "activation": "ReLU",
        "layer_count": layer_idx
    }
    
    model_params["metadata"] = model_metadata
    
    with open(f"{output_dir}/model_weights.json", "w") as f:
        json.dump(model_params, f, indent=2)
    
    # 4. 導出測試向量（用於驗證）
    test_input = np.random.randn(1, 17).astype(np.float32)
    test_input_scaled = scaler.transform(test_input)
    
    with torch.no_grad():
        test_output = model(torch.tensor(test_input_scaled, dtype=torch.float32))
    
    test_data = {
        "input_raw": test_input.tolist(),
        "input_scaled": test_input_scaled.tolist(),
        "expected_output": test_output.numpy().tolist()
    }
    
    with open(f"{output_dir}/test_vectors.json", "w") as f:
        json.dump(test_data, f, indent=2)
    
    print(f"✅ 模型導出完成！")
    print(f"📁 輸出目錄: {output_dir}")
    print(f"📄 檔案:")
    print(f"   - scaler_params.json: StandardScaler參數")
    print(f"   - model_weights.json: 神經網絡權重")
    print(f"   - test_vectors.json: 測試向量")

# 使用範例：
if __name__ == "__main__":
    # 假設你已經有訓練好的模型和scaler
    # model = your_trained_model
    # scaler = your_trained_scaler
    
    # export_vtr_model(model, scaler)
    
    print("請在你的 vtr_model.ipynb 最後加入以下代碼：")
    print()
    print("# 導出模型")
    print("exec(open('model_export_script.py').read())")
    print("export_vtr_model(model, scaler)")