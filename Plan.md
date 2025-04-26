# WRO Code TODO

## Journal
https://hedgedoc.timon.ch/P4v4aR35QGK7QTs8tbAWeA

## Localization

Mir wennd üsi position based on 5 linie bestimme: grenze usse, grenze inne, grenze hinne, orangi und blaui linie. Das heisst schritt 1 isch die linie relyably chönne erkenne:
1. Für jedi farb (orange, blau, wiis, farb vo usserhalb vo de grenze (momentan blau aber eventually schwarz aber das sött mer denn eifach i dere funktion chönne ändere)) e funktion wo en pixel nimmt und seit ob de pixel die farb hett. 

2. Zum das teste e funktion wo es bild nimmt und e funktion wo en pixel nimmt und seit ob er vomene farb isch und denn alli pixel wo als die farb erkennt werded überschriibt mit irgendmene marker farb iwie hellgrüe oderso und nocher das bild saved. denn chömmer de robbi a verschiede örter platziere und luege ob mir happy sind was alles als die farb erkennt wird und wa ned. E bispil vomene C++ funktion wo e funktion als argument nimmt: 

```
void call_until_false( bool (*f) (int) ) {
    for (int i = 0; f(i); ++i);
}
 ```

Für orange und blau isch das scho alles, aso det mue mer nur für jede pixel denn die funktion calle. Für d‘grenze isches no chli komplizierter. 

3. Min vorschlag für det isch dass mer für jede pixel luegt obs ide nochburschaft (mir chönnd zb mol 3x3 gebiet rund um de pixel neh) einigi pixel mit farb usserhalb de grenze und einigi wiisi hett. falls ja zellt de als grenzpixel. denn müemmer no bestimme zu welere grenze er ghört. Für das chönnd mir de image gradient (chönnd ihr online sueche) bestimme und denn je nach richtig es zu de 3 grenze zueordne. Das heisst i wür e funktion mache wo es bild nimmt und nocher 3 vector vo points returned, für jedi grenze eine mit allne points wo mer det druuf gfunde hend.

4. Denn wemmer jo based on allne pünkt wo mer uf mene linie gfunde hend e linie uf em screen finde wo mir als die linie zelled. Damit mir det di glich funktion chönnd bruche für orange, blau und d‘grenze sött die eifach en vektor vo pünkt neh wo uf em segment sötted si. die returned denn entweder e linie oder ned wenn es ned gnueg pünkt het dass mer es als linie zelle chan. das macht mer am schönste mit mene std::optional. De algorithmus wie mer vo vektor vo pünkt zu segment chunnt hani im whatsapp chat beschriebe. aso ebe eifach einigi mol 2 random pünkt neh und denn d‘linie zwüsched dene berechne und nocher angle und offset merke für jedi vo dene linie und denn das neh wo am hüfigste so ungefähr cho isch. det hani mol gseit gha dass mer maybe stigig und y-achse abschnitt sött neh aber das isch chli ass well zb e linie mit stigig 100 und 200 sind recht ähnlich aber eini mit 0 und 1 sind echt unterschiedlich aber die werded als viel ähnlicher zellt. mer sött glaub ebe angle aso arctan vo de stigig und signed distanz zumene fixpunkt zb s‘zentrum vom screen neh. nocher zum s‘neighborhood finde wo am hüfigste vorcho isch hani gseit cha mer de median neh. au das isch echli ungünstig well wenn de winkel recht nöch a 0 isch denn cha‘s sii dass d‘hälfti 0° hett und dhälfti 179° und denn de median iwie 90° isch. mer chönnti das scho iwie patche aber i glaub en cleanere approach isch probiere das 5° oderso window finde wo es am meiste drin hett. das cha mer mit scanline mache (google oder mich pinge). wenn mer assumed dass es nur eis cluster git (wa e reasonable assumption sött sii) cha mer das unabhängig für angle und offset mache und nocher als linie eifach d‘kombi neh.

5. Zum d'linie erkennig teste wär's wieder nice wenn mer e funktion hetted wo d'linie uf em frame iizeichne wür und nocher als png speichere wür.

6. Denn müend mir e koordinatesystem uf em spielfeld definiere. Es chunnt ned so drufah wa mir nehmed, mir chönnd eifach öppis neh wo einigermasse natürlich schiint. Mir müend denn alli mögliche linie i dem koordinatesystem locate. Aso das heisst alli grenze und di orange und die blaue linie d'end und startpünkt koordinate müend mir bestimme. Wenn mir aber es einigermasse schlaus koordinatesystem wähled sötted das einigermasse schöni wert sii. Denn sött mer iwelchi konventione iiführe zum linie identifiere im code aso zb sie durenummeriere und denn e funktion mache wo mer linie nummer cha geh und es git eim 3d start und endpunkt vo dere linie.

7. Mer müend die sogenannte intrinsics vo de kamera messe. Mir müend einersiits luege ob d'kamera viel distortion hett. Falls ja isch da chli rip und macht einiges komplizierter. Wär aber jetzt ned the end of the world. Falls ned tiptop denn müemmer vor allem no s'FOV (vertikal oder horizontal isch egal, s'andere cha mer jo denn uusrechne mit em aspect ratio) messe. Zuesätzlich sött mer de "tilt angle" vo de kamera uf em robi messe aso mit welem winkel d'kamera abeluegt, was jo fix isch. Mer mue au d'höchi vo de kamera über em bode messe, isch jo au fix. Wa mir au assume werded dass es fix isch isch roll aso ob d'kamera halt roll hett. Das sött mer no schnell verifiziere dass das 0 isch und sus d'kamera chli drülle.

7. En helper wo mer denn brucht wird isch denn based uf mene linie uf em screen e plane bestimme wo zu dere linie korrespondiert. Aso wenn ihr eu vorstelled wenn mer e linie uf mene screen unprojected in 3d giits e plane wo die pünkt möglicherwiis chönnd hercho. Die plane bruchemer. Das heisst mer mue e funktion mache wo mer d'position und rotation vo de kamera giit und e linie uf em screen und es git eim e plane wo zu dere linie korrespondiert. D'planes söttemer parametrisiere mit mene normalevektor wo sie dur de kameraursprung hend (mir wüssed jo dass sie dur de kameraursprung gönnd).

8. Denn en schritt isch für jedi linie uf em screen usefinde zu welere linie sie korrespondiert uf em feld. Für das isch min vorschlag d'position vom letschte frame als reference neh und so tue als ob das di jetztig position wär und denn luege welli linie uf em feld am beste matched. Aso wenn mer e linie uf mene screen hett korrespondiert das jo ebe zumene plane in 3d wo die pünkt chönnd hercho. denn cha mer eifach dur alli linie wo theoretisch chönnted matche (aso zb alli orange), und d'distanz vom start und endpunkt zu dere plane berechne und nocher de candidate neh wo am distanze am chliinste sind. Mer chönnt das definitiv andersch löse zb de quadrant wo mir üs ufhaltet tracke und denn based uf dem matche. Aber wie i mir's gad vorstell ishc das müehsamer. Wa a mim weg evtl es problem ishc das di orange und blaue linie wo gegeüber lieged einigermasse kolinear sind und es denn mir viel pech die verwechsle chönnt. Das heisst mer müesst maybe no en check adde dass s'segment zwüsched start und endpunkt zumindest remotely im field of view isch. Aso endresultat vo dere ufgob wär e funktion wo mer di letschti position git, e linie uf em screen  und seit welle typ (aso orange, blau, grenzi inne, grenze usse/hinne) und sie eim de identifier zu de linie uf em feld git wo am beste matched.

9. Sobald mer gmatcht hett muess mer no die position und rotation finde wo macht dass d'distanze vo de start und endpünkt zude planes am chlinste sind. Für das definiert mer theoretisch e "loss function" wo heisst wie wiit weg sie sind und macht denn gradient descent. Mir müend aber ned explizit d'loss function definiere. Was üsi loss function isch isch eifach d'summe vo allne quadrat vo de distanze vo de start und endpünkt zude planes. De gradient vo dem uf d'kamera parameter (aso xy position und xy rotation) berechne isch eingiermasse guet machbar. Gradient uf position isch super easy. Gradient uf rotation isch au ned so bad, mer cha gradient uf di 3 komponente vom normalevektor vo de plane berechne und nocher chain rule zum vo dem uf de gradient uf d'rotation cho. Das isch jetzt ned so guet erklärt do aber i chas au besser irl erkläre wenn ihr das actually mache wennd. Aso endresultat vo dem wär e funktion wo e plane (aso eifach en normalevektor) und start uf endpünkt vode korrepondierende linie uf em feld nimmt und gradients uf d'position und rotation vo de kamera berechnet.

10. Nocher mue mer mit de gradients denn de actual gradient descent mache. Das ishc relativ easy, alles wo mer do überlege mue isch wenn hört mer uf und was isch d'learning rate. Als startpunkt für de gradient descrent nehmemer eifach d'position vom letschte frame. Für das wür mer ned e separati funktion mache sondern eifach i e grossi funktion fetze wo dnen au di vorherige steps called. Das heisst endresultat wär die funktion. Spezifisch stelli mir vor dass die folgendes nimmt: di letschti position und es std::optional für jede typ vo linie aso orange, blau, grenze inne, grenze hinne, grenze usse. Mer würed probably en struct defür mache dass mer das cleanly vom bildbearbeitsteil returne chan. I bin theoretisch offe für change aber das separiert d'bildverarbeitig und d'3d geo relativ nicely. Aso üsi final localization funktion wür denn ungefähr so usgeh:

```
CameraPosition locate(const CameraPostion &lastPosition, const Frame &frame) {
    ScreenLines lines = findLines(frame);
    CameraPosition updated = optimizePosition(lastPosition, lines);
    return updated;
}
```

ScreenLines wär en struct mit ebe dene 5 optionals. CameraPosition wär en struct mit x,y position und xy rotation vo de kamera.

Das wärs scho. 


## Fahre

I ha mir das echli überleit und bin uf e design cho woni actually ziemli cool find. Es isch based on "modes" (isch mir gad kein bessere name iigfalle). Wie das funktioniere wür wär so: De roboter kennt viel verschiedeni modes. Ein mode bestoht us mene funktion wo am ahfang ufgruefe wird wo denn returned was gmacht werde sött. Die funktion chunnt position und en frame über und returned en vektor vo waypoints wo sötted abfahre werde als nächst und en de mode wo de roboter sött mache wenn er fertig isch mit die pünkt abfahre. En mode cha denn zb sii "startSegment", wo de roboter sött drin sii wenn er eis vo de 4 segment (aso siite vom spielfeld) ahfangt. De mode chönnt denn zb luege obs ide erste reihe vo spots es liechtsignal hett und falls ja en waypoint returne zum um das umefahre und falls ned en waypoint e bitzeli wiiter vorne returne. Und denn de nächste mode chan in beide fäll iwie "middleSegment" sii. Aso das mit de modes findi halt nice well so du chashc ei task vom robi separate i ein modus und du chashc denn easy mache dass verschiedeni modes nocher wieder s'gliche mached so kei ahnig i cha gad ned erkläre aber i find das design so geil.

I bin mir no echli unsicher ob mir wennd mache dass waypoints nur e target position beinhaltet oder au e target rotation. Vorteil vo nur location isch dass es eifacher isch zum programmiere zum die waypoints denn erreiche probiere. Vorteil vo rotation isch dass es weniger fest drufahchunnt vo wo mer herchunnt. S'ding isch dass es maybe ned immer möglich isch zum die target rotation easily erreiche und es denn glich wieder drufah chunnt wo mer hercho ishc. I schlag vor mol nur target position implementiere und denn luege ob mir issues hitted und denn cha mer immer no rotation adde.

Aso gits folgendi ufgobe für s'fahre. 

11. Überlege welli modes mir alles wennd und was sie mached. 

12. Die modes implementiere.

13. De code mache wo denn d'befehel vo de modes usführt aso d'waypoints abfahrt. Wenn mer nur position waypoints macht isch das eingermasse eifach. Mer cha eifach luege wie isch die momentan rotation vom robi (aso sött jo s'gliche sii wie d'rotation vo de cam, assuming mir mounted sie schön grad) und wa isch d'richtig zum nächste waypoint und denn je noch dem links oder rechts stüüre. Für s'stüüre schlagi vor dass mir en PD controller nehmed wo de input de unterschied vo target rotation (aso rotation zum nächste waypoint) und momentani rotation isch und de output steering angle isch. 

Aso wieder die most high level funktionalität in code wie i's mir vorstelle chönnt:

```
queue<Waypoint> waypoints;
Mode nextMode;
while (true) {
    // Localization code goes here, assume we have position, frame.

    if (waypoints.front().reached(position))
        waypoints.pop();
    }
    if (waypoints.empty()) {
        bool isTerminal;
        // C++ is inferior to Rust.
        std::tie(waypoints, nextMode, isTerminal) = nextMode.plan(position, frame);
        if (isTerminal) break;
    }
    Commands commands = getCommands(position, waypoints.front());
    sendCommands(commands);
}
```

I bin noned sicher ob i en Mode typ wür mache. Aso mer brucht eigentli nur d'plan funktion für jede mode. Denn chönnt mer easy au eifach en function pointer neh. 
Update: probably isches au schlauer wenn plan es optional mit waypoints und nextMode returned und none bedütet denn dass es terminal isch.
