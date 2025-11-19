import os
import sys
from PIL import Image

def convert_image_to_rgb666(image_path, output_path):
    """
    Converte uma imagem para o formato binário para ILI9488 (3 bytes por pixel).
    CORREÇÃO: Envia RGB888 (0-255) completo. O display pega os 6 bits superiores (RGB666).
    """
    try:
        img = Image.open(image_path).convert('RGB')
        width, height = img.size
        pixels = img.load()

        print(f"Convertendo '{image_path}' ({width}x{height}) -> '{output_path}'")

        with open(output_path, 'wb') as f:
            for y in range(height):
                for x in range(width):
                    r_8bit, g_8bit, b_8bit = pixels[x, y]
                    
                    # NÃO fazemos o shift (>> 2). 
                    # Enviamos o byte cheio (0-255) para garantir brilho máximo.
                    # O ILI9488 em modo 18-bits vai ler os bits mais significativos (MSB).
                    
                    f.write(r_8bit.to_bytes(1, byteorder='little'))
                    f.write(g_8bit.to_bytes(1, byteorder='little'))
                    f.write(b_8bit.to_bytes(1, byteorder='little'))

    except Exception as e:
        print(f"ERRO ao converter '{image_path}': {e}")

def process_folder(input_dir, output_dir):
    """
    Varre o diretório de entrada, converte todas as imagens .png
    e salva os arquivos .bin no diretório de saída.
    """
    
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        print(f"Diretório de saída criado: '{output_dir}'")

    found_images = 0
    for filename in os.listdir(input_dir):
        if filename.lower().endswith('.png'):
            found_images += 1
            
            input_path = os.path.join(input_dir, filename)
            
            output_filename = os.path.splitext(filename)[0] + '.bin'
            output_path = os.path.join(output_dir, output_filename)
            
            convert_image_to_rgb666(input_path, output_path)

    if found_images == 0:
        print(f"Nenhum arquivo .png encontrado em '{input_dir}'")
    else:
        print(f"\nConcluído! {found_images} imagens convertidas e salvas em '{output_dir}'.")

# --- Início da Execução ---
if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Uso: python convert_folder.py <diretorio_de_entrada> <diretorio_de_saida>")
        print("Exemplo: python convert_folder.py images/ bin/")
        sys.exit(1)

    input_folder = sys.argv[1]
    output_folder = sys.argv[2]
    
    if not os.path.isdir(input_folder):
        print(f"Erro: O diretório de entrada '{input_folder}' não existe.")
        sys.exit(1)

    process_folder(input_folder, output_folder)