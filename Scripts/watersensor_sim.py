import serial
import time
import struct

# Serielle Schnittstelle konfigurieren (Anpassen an deine Einstellungen)
ser = serial.Serial(
    port='/dev/ttyACM0',       # Port anpassen (z. B. '/dev/ttyUSB0' auf Linux)
    baudrate=115200,   # Baudrate anpassen
    timeout=1
)

# Paket-Counter (startet bei 0 und wird inkrementiert)
packet_counter = 0

def calculate_checksum(data):
    """Berechnet eine einfache Checksumme (XOR aller Bytes)."""
    checksum = 0
    for byte in data:
        checksum ^= byte
    return checksum

def send_packet(measurement_value):
    """Sendet ein Datenpaket mit der definierten Struktur."""
    global packet_counter

    # Paket aufbauen:
    # 1. Paket-Counter (1 Byte)
    # 2. Messwert (4 Byte, little-endian)
    # 3. Feste Bytes 0xC0DE (2 Byte, big-endian)
    # 4. Checksum (1 Byte)

    # Paket-Counter (1 Byte)
    counter_byte = struct.pack('B', packet_counter)

    # Messwert (4 Byte, little-endian)
    measurement_bytes = struct.pack('<i', measurement_value)

    # Feste Bytes 0xC0DE (2 Byte, big-endian)
    fixed_bytes = struct.pack('>H', 0xC0DE)

    # Daten für Checksumme vorbereiten
    data_for_checksum = counter_byte + measurement_bytes + fixed_bytes

    # Checksum berechnen
    checksum = calculate_checksum(data_for_checksum)

    # Checksum anhängen
    packet = data_for_checksum + struct.pack('B', checksum)

    # Paket senden
    ser.write(packet)

    # Paket-Counter erhöhen (modulo 256, da 1 Byte)
    packet_counter = (packet_counter + 1) % 256

# Hauptschleife: Alle 50 ms ein Paket senden
try:
    while True:
        # Beispiel-Messwert (kann durch echte Messung ersetzt werden)
        measurement = 12345  # Hier deinen echten Messwert einsetzen

        send_packet(measurement)
        time.sleep(0.05)  # 50 ms Verzögerung

except KeyboardInterrupt:
    print("\nSkript beendet.")
    ser.close()
