import os
import re
import glob

# Dossier contenant les CSV
INPUT_DIR = "./"

# Dossier de sortie
OUTPUT_DIR = "./merged"
os.makedirs(OUTPUT_DIR, exist_ok=True)

# Regex pour reconnaître les fichiers
pattern = re.compile(r"_dev_ttyACM(\d+)_Measurements_(\d+)\.csv")

# Regrouper les fichiers par ttyACM
groups = {}

for filepath in glob.glob(os.path.join(INPUT_DIR, "*.csv")):
    filename = os.path.basename(filepath)

    match = pattern.match(filename)
    if match:
        acm_id = match.group(1)
        index = int(match.group(2))

        groups.setdefault(acm_id, []).append((index, filepath))

# Fusion
for acm_id, files in groups.items():

    # Trier par numéro de mesure
    files.sort(key=lambda x: x[0])

    output_file = os.path.join(
        OUTPUT_DIR,
        f"ttyACM{acm_id}_merged.csv"
    )

    with open(output_file, "w", encoding="utf-8") as outfile:

        # Header optionnel
        outfile.write("freq,val1,val2,timestamp\n")

        for _, filepath in files:

            with open(filepath, "r", encoding="utf-8") as infile:

                for line in infile:
                    line = line.strip()

                    if not line:
                        continue

                    try:
                        # Exemple :
                        # b'37473.68,1715.57,-9.14\n',Sun Apr 26 15:08:49 2026

                        data_part, timestamp = line.split("',", 1)

                        # Nettoyage du b'
                        data_part = data_part.replace("b'", "")

                        # Suppression du \n
                        data_part = data_part.replace("\\n", "")

                        outfile.write(f"{data_part},{timestamp}\n")

                    except Exception as e:
                        print(f"Erreur ligne : {line}")
                        print(e)

    print(f"Fusion terminé : {output_file}")
