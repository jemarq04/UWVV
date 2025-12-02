import os
import filecmp
import configparser

def main():
    settings = configparser.ConfigParser()
    settings.read("data/XPOG.cfg")

    basepath = settings.get("DEFAULT", "basepath")
    if not os.path.isdir(basepath):
        print(f"error: invalid base path: {basepath}")
        exit(1)
    
    for era in settings.sections():
        for POG in os.listdir(basepath):
            if POG in settings[era]:
                era_path = os.path.join(basepath, POG, settings[era]["name"])
                latest = os.path.join(era_path, "latest")
                current = os.path.join(era_path, settings[era][POG])

                is_latest = os.path.isdir(current)
                for f in os.listdir(latest):
                    if not is_latest:
                        break
                    if not os.path.isfile(os.path.join(latest, f)) or ".json" not in f:
                        continue

                    is_latest = os.path.isfile(os.path.join(current, f)) and \
                            filecmp.cmp(os.path.join(latest, f), os.path.join(current, f))

                if not is_latest:
                    print(f"{era} {POG} corrections have been updated!")
                    for opt in os.listdir(era_path):
                        if os.path.isdir(os.path.join(era_path, opt)):
                            print(f" - {opt}")
                    print()
    print("Done.")

if __name__ == "__main__":
    main()
