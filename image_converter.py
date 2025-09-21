# converter_imagem.py
import sys
from PIL import Image

def convert_image_to_c_source(image_path, variable_name):
    try:
        img = Image.open(image_path)
    except FileNotFoundError:
        print(f"Erro: O ficheiro de imagem '{image_path}' não foi encontrado.")
        return

    # Garante que a imagem está no formato RGB
    img = img.convert('RGB')
    
    width, height = img.size
    pixels = list(img.getdata())

    # --- Cria o ficheiro .c ---
    c_file_name = f"{variable_name}.c"
    with open(c_file_name, "w") as f:
        print(f"A gerar {c_file_name}...")
        
        f.write('#include <stdint.h>\n\n')
        f.write(f'const struct {{\n')
        f.write(f'  unsigned int   width;\n')
        f.write(f'  unsigned int   height;\n')
        f.write(f'  unsigned int   bytes_per_pixel;\n')
        f.write(f'  const char    *comment;\n')
        f.write(f'  const uint8_t  pixel_data[{width} * {height} * 3];\n')
        f.write(f'}} {variable_name}_map = {{\n')
        f.write(f'  {width}, {height}, 3,\n')
        f.write(f'  "Imagem convertida com script Python",\n')
        f.write(f'  {{\n')

        # Escreve os dados dos píxeis
        for i, p in enumerate(pixels):
            r, g, b = p
            f.write(f'0x{r:02x}, 0x{g:02x}, 0x{b:02x}, ')
            if (i + 1) % 16 == 0: # Quebra de linha a cada 16 píxeis para legibilidade
                f.write('\n')
        
        f.write(f'\n  }}\n')
        f.write(f'}};\n')
        print(f"{c_file_name} gerado com sucesso!")

    # --- Cria o ficheiro .h ---
    h_file_name = f"{variable_name}.h"
    with open(h_file_name, "w") as f:
        print(f"A gerar {h_file_name}...")
        
        f.write(f'#ifndef INC_{variable_name.upper()}_H_\n')
        f.write(f'#define INC_{variable_name.upper()}_H_\n\n')
        f.write('#include <stdint.h>\n\n')
        f.write(f'extern const struct {{\n')
        f.write(f'  unsigned int   width;\n')
        f.write(f'  unsigned int   height;\n')
        f.write(f'  unsigned int   bytes_per_pixel;\n')
        f.write(f'  const char    *comment;\n')
        f.write(f'  const uint8_t  pixel_data[{width} * {height} * 3];\n')
        f.write(f'}} {variable_name}_map;\n\n')
        f.write(f'#endif /* INC_{variable_name.upper()}_H_ */\n')
        print(f"{h_file_name} gerado com sucesso!")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Uso: python converter_imagem.py <caminho_da_imagem_entrada> <nome_da_variavel_saida>")
        print("Exemplo: python converter_imagem.py Logo.png Logo300")
    else:
        input_image = sys.argv[1]
        output_name = sys.argv[2]
        convert_image_to_c_source(input_image, output_name)