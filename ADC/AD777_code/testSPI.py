import spidev
import time

# Configuration
SPI_BUS = 0
SPI_DEVICE = 0  # CS0
SPI_SPEED = 1000000  # 1 MHz pour commencer
SPI_MODE = 0

def test_spi():
    spi = spidev.SpiDev()
    spi.open(SPI_BUS, SPI_DEVICE)
    spi.max_speed_hz = SPI_SPEED
    spi.mode = SPI_MODE
    spi.bits_per_word = 8

    print(f"SPI ouvert: bus={SPI_BUS}, device={SPI_DEVICE}, speed={SPI_SPEED}Hz, mode={SPI_MODE}")

    # Test 1 : envoi d'un octet simple
    print("\n--- Test 1: envoi octet simple ---")
    tx = [0xAA]
    rx = spi.xfer2(tx)
    print(f"TX: {[hex(b) for b in tx]}")
    print(f"RX: {[hex(b) for b in rx]}")

    # Test 2 : envoi de plusieurs octets
    print("\n--- Test 2: envoi multi-octets ---")
    tx = [0x80, 0x00, 0x00, 0x00]
    rx = spi.xfer2(tx)
    print(f"TX: {[hex(b) for b in tx]}")
    print(f"RX: {[hex(b) for b in rx]}")

    # Test 3 : loopback - MOSI connecté à MISO
    print("\n--- Test 3: loopback (connecte MOSI à MISO) ---")
    print("Appuie sur Entrée quand MOSI est connecté à MISO...")
    input()
    tx = [0x01, 0x02, 0x03, 0x04, 0x05]
    rx = spi.xfer2(tx)
    print(f"TX: {[hex(b) for b in tx]}")
    print(f"RX: {[hex(b) for b in rx]}")
    if tx == rx:
        print("✓ Loopback OK - SPI fonctionne correctement")
    else:
        print("✗ Loopback FAIL - problème SPI détecté")

    # Test 4 : stress test
    print("\n--- Test 4: stress test (1000 transferts) ---")
    errors = 0
    tx = [0xAA, 0x55, 0xAA, 0x55]
    for i in range(1000):
        rx = spi.xfer2(tx)
        if rx != tx:  # valide seulement en loopback
            errors += 1
    print(f"1000 transferts effectués, erreurs loopback: {errors}")

    spi.close()
    print("\nSPI fermé.")

if __name__ == "__main__":
    try:
        test_spi()
    except Exception as e:
        print(f"Erreur: {e}")
        print("Vérifie que SPI est activé: sudo raspi-config -> Interface Options -> SPI")
