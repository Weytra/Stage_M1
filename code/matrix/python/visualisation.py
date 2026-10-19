import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("resultat/seq_calsique/result.csv")

# Une courbe par optimisation
for opt in df["opt"].unique():
    sub = df[df["opt"] == opt]
    plt.plot(sub["size"], sub["avg_time"], marker="o", label=opt)

plt.xlabel("Taille de matrice (n)")
plt.ylabel("Temps moyen (s)")
plt.title("Performance de la multiplication matricielle")
plt.legend()
plt.grid(True)
plt.show()
