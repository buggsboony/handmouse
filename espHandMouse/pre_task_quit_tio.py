import os
import time
Import("env")

def kill_tio_cleanly(target, source, env):
    # On cherche si 'tio' tourne
    # pgrep -x cherche exactement le nom du processus
    check_tio = os.system("pgrep -x tio > /dev/null")
    
    if check_tio == 0:
        print("\n--- [TIO] Fermeture du processus en cours... ---")
        # Signal 15 (SIGTERM) : demande une fermeture propre
        os.system("pkill -15 -x tio || true")
        
        # On attend un peu que le noyau Linux libère le verrou sur /dev/ttyACM0
        time.sleep(0.8)
        print("--- [TIO] Port libéré. ---\n")
    else:
        print("\n--- [TIO] Aucun processus actif trouvé. ---")

env.AddPreAction("upload", kill_tio_cleanly)