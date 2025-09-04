#!/bin/bash

echo ""
echo "🛠️  Iniciando instalação do filtro Epson TM-T20II"
echo "-----------------------------------------------"

# Verifica dependências
echo "🔍 Verificando pacotes necessários..."
sudo apt-get update
sudo apt-get install -y libcups2-dev libcupsimage2-dev g++ cups cups-client

# Compila o filtro
echo ""
echo "⚙️  Compilando o filtro rastertozj..."
make clean
make

./install

if [ ! -f rastertozj ]; then
    echo "❌ Erro: binário rastertozj não foi gerado. Verifique o código-fonte."
    exit 1
fi

# Detecta diretório de filtros do CUPS
FILTER_DIR=$(cups-config --serverbin)/filter

echo ""
echo "📁 Instalando o filtro em: $FILTER_DIR"
sudo cp rastertozj "$FILTER_DIR/"
sudo chmod +x "$FILTER_DIR/rastertozj"

echo ""
echo "✅ Filtro instalado com sucesso!"

# Sugestão de configuração
echo ""
echo "🖨️  Para configurar a impressora no CUPS:"
echo "1. Acesse http://localhost:631"
echo "2. Vá em Administration > Add Printer"
echo "3. Selecione sua Epson TM-T20II (USB ou rede)"
echo "4. Escolha driver genérico (Raw ou ESC/POS)"
echo "5. Nome sugerido: Epson_TM_T20II"
echo "6. Finalize e imprima uma página de teste"

# Alternativa via terminal
echo ""
echo "💡 Ou configure via terminal com:"
echo "lpadmin -p Epson_TM_T20II -E -v usb://EPSON/TM-T20II -m raw"

echo ""
echo "🚀 Instalação concluída."
