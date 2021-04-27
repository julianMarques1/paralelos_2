ejecutar() {
  echo "Entrada = $1:"
  ./entregaSecuencial $1
}

echo 'Output hogareño:' > output.txt

ejecutar 512
ejecutar 1024
ejecutar 2048
ejecutar 4096
