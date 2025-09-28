# convert_folder.py
from PIL import Image
import sys
import os

def convert_to_rgb565(r, g, b):
    """Converte uma cor de 24 bits (RGB888) para 16 bits (RGB565)."""
    return ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3)

def process_image(input_path, output_path):
    """
    Converte uma única imagem PNG para um ficheiro binário bruto no formato RGB565.
    """
    try:
        image = Image.open(input_path).convert('RGB')
    except Exception as e:
        print(f"Erro ao abrir a imagem '{input_path}': {e}")
        return

    width, height = image.size
    print(f"Convertendo '{os.path.basename(input_path)}' ({width}x{height})...")

    with open(output_path, 'wb') as f_out:
        for y in range(height):
            for x in range(width):
                r, g, b = image.getpixel((x, y))
                rgb565 = convert_to_rgb565(r, g, b)
                f_out.write(rgb565.to_bytes(2, 'little'))

def convert_folder(input_folder, output_folder):
    """
    Converte todos os ficheiros .png numa pasta de entrada para ficheiros .bin na pasta de saída.
    """
    # Verifica se a pasta de entrada existe
    if not os.path.isdir(input_folder):
        print(f"Erro: A pasta de entrada '{input_folder}' não existe.")
        return

    # Cria a pasta de saída se ela não existir
    os.makedirs(output_folder, exist_ok=True)
    
    print(f"Procurando por imagens .png em '{input_folder}'...")
    converted_count = 0

    # Itera sobre todos os ficheiros na pasta de entrada
    for filename in os.listdir(input_folder):
        # Verifica se o ficheiro é um .png
        if filename.lower().endswith(".png"):
            input_path = os.path.join(input_folder, filename)
            
            # Cria o nome do ficheiro de saída
            base_name = os.path.splitext(filename)[0]
            output_path = os.path.join(output_folder, base_name + ".bin")
            
            # Converte a imagem
            process_image(input_path, output_path)
            converted_count += 1
            
    if converted_count == 0:
        print("Nenhum ficheiro .png encontrado para converter.")
    else:
        print(f"\nConversão concluída! {converted_count} ficheiro(s) convertido(s) para a pasta '{output_folder}'.")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Uso: python convert_folder.py <pasta_de_entrada> <pasta_de_saida>")
        print("Exemplo: python convert_folder.py ./minhas_imagens ./bin_para_sd")
        sys.exit(1)

    input_dir = sys.argv[1]
    output_dir = sys.argv[2]
    
    convert_folder(input_dir, output_dir)