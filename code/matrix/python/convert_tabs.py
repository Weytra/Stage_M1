# convert_tabs.py
# Remplace le caractère spécial § par un vrai TAB dans un fichier Makefile

INPUT = "Makefile.src"
OUTPUT = "Makefile"

with open(INPUT, "r") as f:
    content = f.read()

# Remplacement du caractère spécial par un vrai TAB ASCII
content = content.replace("<TAB>", "\t")

with open(OUTPUT, "w") as f:
    f.write(content)

print("Makefile généré avec de vrais TAB →", OUTPUT)
