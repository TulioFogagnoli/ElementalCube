import os
import sys
from PIL import Image

# Cor Chave para transparência no STM32 (Magenta Puro)
# Isso deve bater com o #define no seu código C
COLOR_KEY_R = 255
COLOR_KEY_G = 0
COLOR_KEY_B = 255

def convert_image_to_rgb666_clean(image_path, output_path):
    """
    Converte PNG para BIN (RGB888) tratando bordas semitransparentes.
    Usa limiar no canal Alpha para evitar bordas 'sujas' (anti-aliasing).
    """
    try:
        # IMPORTANTE: Abre como RGBA para ler a transparência
        img = Image.open(image_path).convert('RGBA')
        width, height = img.size
        pixels = img.load()

        # MANTIDO: Printando o tamanho da imagem conforme solicitado
        print(f"Convertendo '{image_path}' ({width}x{height}) -> '{output_path}'")

        with open(output_path, 'wb') as f:
            for y in range(height):
                for x in range(width):
                    r, g, b, a = pixels[x, y]
                    
                    # LÓGICA DE CORTE SECO (THRESHOLD)
                    # Se o pixel for mais transparente que opaco (Alpha < 128),
                    # forçamos ele a ser a Cor Chave (Magenta Puro).
                    # Isso elimina os pixels de borda misturados que causavam o contorno rosa.
                    if a < 128:
                        r = COLOR_KEY_R
                        g = COLOR_KEY_G
                        b = COLOR_KEY_B
                    
                    # Escreve os bytes (RGB 888)
                    # O display pega os 6 bits mais significativos de cada byte
                    f.write(r.to_bytes(1, byteorder='little'))
                    f.write(g.to_bytes(1, byteorder='little'))
                    f.write(b.to_bytes(1, byteorder='little'))

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
            
            convert_image_to_rgb666_clean(input_path, output_path)

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