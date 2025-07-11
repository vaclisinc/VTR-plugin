import json
import numpy as np
import torch
import torch.nn as nn
from sklearn.preprocessing import StandardScaler
import pandas as pd
from sklearn.model_selection import train_test_split

# Define the same model structure as in nn.py
class SimpleMLP(nn.Module):
    def __init__(self, in_dim, hidden=64, out_dim=5):
        super().__init__()
        self.net = nn.Sequential(
            nn.Linear(in_dim, hidden),
            nn.ReLU(),
            nn.Linear(hidden, hidden),
            nn.ReLU(),
            nn.Linear(hidden, out_dim)
        )
    def forward(self, x):
        return self.net(x)

def load_exported_model():
    """Load the exported model weights and scaler parameters"""
    
    # Load model weights
    with open('exported_model/model_weights.json', 'r') as f:
        weights_data = json.load(f)
    
    # Load scaler parameters
    with open('exported_model/scaler_params.json', 'r') as f:
        scaler_data = json.load(f)
    
    # Create model instance
    model = SimpleMLP(17, 64, 5)  # 17 features, 64 hidden, 5 outputs
    
    # Load weights into model
    state_dict = {}
    
    # Layer 0 (first linear layer)
    layer0_weight = torch.tensor(weights_data['layer_0']['weight'], dtype=torch.float32)
    layer0_bias = torch.tensor(weights_data['layer_0']['bias'], dtype=torch.float32)
    state_dict['net.0.weight'] = layer0_weight
    state_dict['net.0.bias'] = layer0_bias
    
    # Layer 1 (second linear layer)
    layer1_weight = torch.tensor(weights_data['layer_1']['weight'], dtype=torch.float32)
    layer1_bias = torch.tensor(weights_data['layer_1']['bias'], dtype=torch.float32)
    state_dict['net.2.weight'] = layer1_weight
    state_dict['net.2.bias'] = layer1_bias
    
    # Layer 2 (output layer)
    layer2_weight = torch.tensor(weights_data['layer_2']['weight'], dtype=torch.float32)
    layer2_bias = torch.tensor(weights_data['layer_2']['bias'], dtype=torch.float32)
    state_dict['net.4.weight'] = layer2_weight
    state_dict['net.4.bias'] = layer2_bias
    
    model.load_state_dict(state_dict)
    model.eval()
    
    # Create scaler
    scaler = StandardScaler()
    scaler.mean_ = np.array(scaler_data['mean'])
    scaler.scale_ = np.array(scaler_data['std'])
    
    return model, scaler

def test_with_training_data():
    """Test the exported model with the same training data"""
    
    print("Loading exported model...")
    model, scaler = load_exported_model()
    
    # Load the same data as in nn.py
    print("Loading training data...")
    features_df = pd.read_csv("audio_features.csv")
    labels_df = pd.read_csv("dataset_labels.csv")
    merged_df = pd.merge(features_df, labels_df, on="file")
    
    # Prepare data (same as nn.py)
    X = merged_df.drop(columns=["file", "EQ_80", "EQ_240", "EQ_2500", "EQ_4000", "EQ_10000"]).values
    y = merged_df[["EQ_80", "EQ_240", "EQ_2500", "EQ_4000", "EQ_10000"]].values
    
    # Use the same train/test split (same random_state=42)
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
    
    # Scale the data using the loaded scaler
    X_train_scaled = scaler.transform(X_train)
    X_test_scaled = scaler.transform(X_test)
    
    print(f"Training data shape: {X_train_scaled.shape}")
    print(f"Test data shape: {X_test_scaled.shape}")
    
    # Test with training data
    print("\n=== Testing with Training Data ===")
    with torch.no_grad():
        X_train_tensor = torch.tensor(X_train_scaled, dtype=torch.float32)
        y_train_pred = model(X_train_tensor).numpy()
    
    # Calculate MSE for training data
    train_mse = np.mean((y_train_pred - y_train)**2)
    print(f"Training MSE: {train_mse:.4f}")
    
    # Test with test data
    print("\n=== Testing with Test Data ===")
    with torch.no_grad():
        X_test_tensor = torch.tensor(X_test_scaled, dtype=torch.float32)
        y_test_pred = model(X_test_tensor).numpy()
    
    # Calculate MSE for test data
    test_mse = np.mean((y_test_pred - y_test)**2)
    print(f"Test MSE: {test_mse:.4f}")
    
    # Test with the specific test vector from test_vectors.json
    print("\n=== Testing with Specific Test Vector ===")
    with open('exported_model/test_vectors.json', 'r') as f:
        test_vectors = json.load(f)
    
    # Get the raw input
    raw_input = np.array(test_vectors['input_raw'][0])
    expected_output = np.array(test_vectors['expected_output'][0])
    
    print(f"Raw input: {raw_input}")
    print(f"Expected output: {expected_output}")
    
    # Scale the input
    scaled_input = scaler.transform([raw_input])
    print(f"Scaled input: {scaled_input[0]}")
    
    # Make prediction
    with torch.no_grad():
        input_tensor = torch.tensor(scaled_input, dtype=torch.float32)
        prediction = model(input_tensor).numpy()[0]
    
    print(f"Model prediction: {prediction}")
    print(f"Difference: {prediction - expected_output}")
    print(f"MSE for this sample: {np.mean((prediction - expected_output)**2):.4f}")
    
    # Show some sample predictions vs actual
    print("\n=== Sample Predictions vs Actual (First 5 training samples) ===")
    for i in range(min(5, len(y_train))):
        print(f"Sample {i+1}:")
        print(f"  Actual:    {y_train[i]}")
        print(f"  Predicted: {y_train_pred[i]}")
        print(f"  Difference: {y_train_pred[i] - y_train[i]}")
        print()

if __name__ == "__main__":
    test_with_training_data()