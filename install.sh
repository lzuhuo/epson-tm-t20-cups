#!/bin/bash

echo ""
echo "🛠️  Instalando filtro Epson TM-T20II"
echo "------------------------------------"


sudo apt-get install libcups2-dev libcupsimage2-dev g++ cups cups-client -y

make

# Verifica se o binário existe
if [ ! -f rastertozj ]; then
    echo "❌ Erro: binário 'rastertozj' não encontrado. Compile com 'make' antes de instalar."
    exit 1
fi

# Detecta diretório de filtros do CUPS
FILTER_DIR=$(cups-config --serverbin)/filter

echo "📁 Instalando filtro em: $FILTER_DIR"
sudo cp rastertozj /usr/lib/cups/filter/
sudo chmod +x /usr/lib/cups/filter/rastertozj

# Instala o arquivo PPD
echo ""
echo "📄 Instalando arquivo PPD personalizado..."
sudo cp tm20.ppd /usr/share/ppd/

# Reinicia o CUPS
echo ""
echo "🔄 Reiniciando o serviço CUPS..."
if command -v systemctl >/dev/null; then
    sudo systemctl restart cups
else
    sudo /etc/init.d/cups restart
fi

echo ""
echo "✅ Instalação concluída com sucesso!"
echo ""
echo "🖨️  Para configurar sua impressora:"
echo "1. Acesse http://localhost:631"
echo "2. Vá em Administration > Add Printer"
echo "3. Selecione sua Epson TM-T20II (USB ou rede)"
echo "4. Escolha 'Provide a PPD file manually' e selecione:"
echo "   /usr/share/ppd/custom/tm20.ppd"
echo "5. Nome sugerido: Epson_TM_T20II"
echo "6. Finalize e imprima uma página de teste"
echo ""
echo "💡 Ou use este comando:"
echo "lpadmin -p Epson_TM_T20II -E -v usb://EPSON/TM-T20II -P /usr/share/ppd/custom/tm20.ppd"
