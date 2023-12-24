# Sample GRC .pro compiler
TEMPLATE = aux
GRC_OUTPUT_DIR = "$$(BUILD_DIR)/var/receiver"
system('mkdir -p "$$GRC_OUTPUT_DIR"')
system('for i in *.grc; do grcc "$i" -o "$$GRC_OUTPUT_DIR"; done')

#Dirty hack to get GNURadio 3.10 working
system('for i in $$GRC_OUTPUT_DIR/*.py; do sed -i "s/.warning(/.warn(/g" "$i"; done')