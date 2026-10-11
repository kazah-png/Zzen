#pragma once
#include <array>
#include <map>
#include <string>
#include <sstream>
namespace zzen::i18n {
inline int language=0;
inline const wchar_t* languages[]={L"Español",L"English",L"日本語",L"简体中文",L"Français",L"हिन्दी"};
// Stable Spanish source keys are independent of the game's recognition rules.
inline const std::map<std::wstring,std::array<std::wstring,6>>& catalog(){
 static const auto data=[] {
  std::map<std::wstring,std::array<std::wstring,6>> out;
  const wchar_t* rows[]={
   L"Sesión|Session|セッション|会话|Session|सत्र",
   L"Diagnóstico|Diagnostics|診断|诊断|Diagnostic|निदान",
   L"Cómo usar Zzen|Getting started|使い方|使用指南|Bien démarrer|शुरुआत करें",
   L"Tu próxima sesión|Your next session|次のセッション|下一次会话|Votre prochaine session|आपका अगला सत्र",
   L"Elige el partido. Zzen se encarga de repetirlo.|Choose a match. Zzen takes care of the repeats.|試合を選ぶだけ。繰り返しは Zzen にお任せ。|选择比赛，Zzen 会自动重复。|Choisissez un match. Zzen se charge de le rejouer.|मैच चुनें। उसे दोहराने का काम Zzen करेगा।",
   L"Diagnóstico en directo|Live diagnostics|ライブ診断|实时诊断|Diagnostic en direct|लाइव निदान",
   L"Pantalla detectada, lectura y acciones del motor.|Screen detection, recognized text and recent actions.|画面の検出、認識した文字、最近の操作。|画面检测、识别文字和最近操作。|Écran détecté, texte reconnu et dernières actions.|पहचानी गई स्क्रीन, पाठ और हाल की कार्रवाइयाँ।",
   L"Todo listo para empezar|Ready when you are|準備ができたら開始|随时可以开始|Prêt à démarrer|शुरू करने के लिए तैयार",
   L"Crónica · Interfaz del juego en español · Teclado y ratón|Chronicle · Game language: Spanish · Keyboard and mouse|クロニクル · ゲーム言語：スペイン語 · キーボードとマウス|编年史 · 游戏语言：西班牙语 · 键盘和鼠标|Chronique · Jeu en espagnol · Clavier et souris|क्रॉनिकल · खेल की भाषा: स्पेनिश · कीबोर्ड और माउस",
   L"Categoría|Category|カテゴリー|类别|Catégorie|श्रेणी",
   L"Rival|Opponent|対戦相手|对手|Adversaire|प्रतिद्वंद्वी",
   L"Dificultad|Difficulty|難易度|难度|Difficulté|कठिनाई",
   L"Partidos|Matches|試合数|比赛场次|Matchs|मैच",
   L"Aparición del rival|Opponent occurrence|同名チームの順番|同名对手序号|Occurrence du rival|प्रतिद्वंद्वी का क्रम",
   L"El número distingue rivales repetidos en la ruta.|Use the occurrence number for repeated opponents.|同じ名前の相手が複数いる場合の順番です。|用于区分路线中重复出现的对手。|Le numéro distingue les adversaires répétés.|एक ही नाम वाले प्रतिद्वंद्वियों को क्रम से चुनें।",
   L"PARTIDOS DE ESTA SESIÓN|SESSION RESULTS|セッション結果|会话结果|RÉSULTATS DE LA SESSION|सत्र के परिणाम",
   L"Sin límite|Unlimited|無制限|不限次数|Illimité|असीमित",
   L"Iniciar sesión|Start session|開始|开始会话|Démarrer|सत्र शुरू करें",
   L"Pausar|Pause|一時停止|暂停|Pause|रोकें",
   L"Reanudar|Resume|再開|继续|Reprendre|जारी रखें",
   L"Detener · F8|Stop · F8|停止 · F8|停止 · F8|Arrêter · F8|समाप्त · F8",
   L"Actualizar catálogo|Refresh catalog|一覧を更新|更新目录|Actualiser la liste|सूची अपडेट करें",
   L"Corregir resultado|Review result|結果を確認|核对结果|Vérifier le résultat|परिणाम जाँचें",
   L"COMPLETADOS|COMPLETED|完了|已完成|TERMINÉS|पूर्ण",
   L"VICTORIAS|WINS|勝利|胜利|VICTOIRES|जीत",
   L"DERROTAS|LOSSES|敗北|失败|DÉFAITES|हार",
   L"EMPATES / PENDIENTES|DRAWS / PENDING|引き分け / 未確認|平局 / 待确认|NULS / À VÉRIFIER|ड्रॉ / लंबित",
   L"Fecha|Date|日時|日期|Date|तारीख",
   L"Resultado|Result|結果|结果|Résultat|परिणाम",
   L"Marcador|Score|スコア|比分|Score|स्कोर",
   L"Victoria|Win|勝利|胜利|Victoire|जीत",
   L"Derrota|Loss|敗北|失败|Défaite|हार",
   L"Empate|Draw|引き分け|平局|Nul|ड्रॉ",
   L"Pendiente|Pending|未確認|待确认|À vérifier|लंबित",
   L"Sin partidos todavía. Abre Crónica en el juego y pulsa Iniciar sesión.|No matches yet. Open Chronicle in the game, then start a session.|まだ試合がありません。ゲームでクロニクルを開き、開始してください。|尚无比赛。在游戏中打开编年史，然后开始会话。|Aucun match. Ouvrez Chronique dans le jeu, puis démarrez une session.|अभी कोई मैच नहीं। खेल में क्रॉनिकल खोलें और सत्र शुरू करें।",
   L"Selecciona un partido para ver su resultado. Los resultados de Gratis duran esta sesión.|Select a match to view its result. Free results last for this session.|試合を選ぶと結果を表示します。無料版の結果は今回のセッションのみ保存されます。|选择比赛查看结果。免费版结果仅保留在本次会话中。|Sélectionnez un match. Les résultats gratuits durent cette session.|परिणाम देखने के लिए मैच चुनें। मुफ़्त संस्करण के परिणाम इसी सत्र तक रहते हैं।",
   L"Los resultados de Gratis se conservan solo durante esta sesión.|Free results are kept only for this session.|無料版の結果は今回のセッションのみ保存されます。|免费版结果仅保留在本次会话中。|Les résultats gratuits sont conservés pour cette session uniquement.|मुफ़्त संस्करण के परिणाम केवल इस सत्र तक रहते हैं।",
   L"Revisa Debug o la captura antes de corregir el resultado.|Check Diagnostics or the screenshot before changing the result.|結果を変更する前に診断または画像を確認してください。|修改结果前请检查诊断或截图。|Vérifiez le diagnostic ou la capture avant de modifier le résultat.|परिणाम बदलने से पहले निदान या स्क्रीनशॉट जाँचें।",
   L"Observar juego|Observe game|ゲームを確認|观察游戏|Observer le jeu|खेल देखें",
   L"Guardar diagnóstico|Save diagnostics|診断を保存|保存诊断|Enregistrer le diagnostic|निदान सहेजें",
   L"Texto reconocido|Recognized text|認識した文字|识别文字|Texte reconnu|पहचाना गया पाठ",
   L"Sin captura|No capture yet|画像なし|暂无截图|Aucune capture|अभी कोई स्क्रीनशॉट नहीं",
   L"La captura del juego aparecerá aquí|The game capture will appear here|ここにゲーム画面が表示されます|游戏截图将显示在这里|La capture du jeu apparaîtra ici|खेल का स्क्रीनशॉट यहाँ दिखेगा",
   L"Listo|Ready|準備完了|就绪|Prêt|तैयार",
   L"Sin acciones|No actions yet|操作なし|暂无操作|Aucune action|अभी कोई कार्रवाई नहीं",
   L"Solo Crónica|Chronicle only|クロニクル専用|仅限编年史|Chronique uniquement|केवल क्रॉनिकल",
   L"F8 detiene la sesión|F8 stops the session|F8 で停止|F8 停止会话|F8 arrête la session|F8 से सत्र समाप्त करें",
   L"Idioma de Zzen|Zzen language|Zzen の言語|Zzen 语言|Langue de Zzen|Zzen की भाषा",
   L"El juego debe estar en español|The game must be set to Spanish|ゲームの言語はスペイン語に設定してください|游戏语言必须设置为西班牙语|Le jeu doit être en espagnol|खेल की भाषा स्पेनिश होनी चाहिए",
   L"El idioma de la aplicación no cambia el idioma reconocido del juego.|App language does not change the required game language.|アプリの言語を変えても、必要なゲーム言語は変わりません。|应用语言不会改变游戏所需的语言。|La langue de l’application ne change pas celle requise pour le jeu.|ऐप की भाषा बदलने से खेल की आवश्यक भाषा नहीं बदलती।",
   L"CONFIGURACIÓN DEL PARTIDO|MATCH SETUP|試合設定|比赛设置|CONFIGURATION DU MATCH|मैच की सेटिंग",
   L"CONTROL DE SESIÓN|SESSION CONTROL|セッション操作|会话控制|CONTRÔLE DE SESSION|सत्र नियंत्रण",
   L"ESPERANDO TU PRIMER PARTIDO|YOUR FIRST MATCH STARTS HERE|最初の試合を始めましょう|从这里开始第一场比赛|VOTRE PREMIER MATCH COMMENCE ICI|आपका पहला मैच यहाँ शुरू होगा",
   L"Crónica|Chronicle|クロニクル|编年史|Chronique|क्रॉनिकल",
   L"Desbloqueo de ruta|Route unlock|ルート解放|解锁路线|Déblocage de route|रूट अनलॉक",
   L"Objetos inusuales|Unusual items|レアアイテム|稀有物品|Objets rares|दुर्लभ वस्तुएँ",
   L"Heroica|Heroic|ヒーロー|英雄|Héroïque|हीरोइक",
   L"Heroica +|Heroic +|ヒーロー +|英雄 +|Héroïque +|हीरोइक +",
   L"Legendaria|Legendary|レジェンド|传奇|Légendaire|लेजेंडरी",
   L"Preparando sesión · F8 para detener|Preparing session · F8 to stop|準備中 · F8 で停止|正在准备 · F8 停止|Préparation · F8 pour arrêter|सत्र तैयार हो रहा है · F8 से समाप्त करें",
   L"Actualizando catálogo de Crónica|Refreshing Chronicle catalog|クロニクル一覧を更新中|正在更新编年史目录|Actualisation du catalogue Chronique|क्रॉनिकल सूची अपडेट हो रही है",
   L"Observando sin enviar entradas|Observing without sending inputs|操作せずに観察中|仅观察，不发送输入|Observation sans envoyer de commandes|बिना इनपुट भेजे निरीक्षण",
   L"Pausado por el usuario|Paused by you|一時停止中|已手动暂停|Mis en pause|आपने रोक दिया है",
   L"Reanudando|Resuming|再開中|正在继续|Reprise|जारी हो रहा है",
   L"Detenido por el usuario|Stopped by you|停止しました|已手动停止|Arrêté|आपने समाप्त किया है",
   L"Detenido · F8|Stopped · F8|停止 · F8|已停止 · F8|Arrêté · F8|समाप्त · F8",
   L"Partido en curso|Match in progress|試合中|比赛进行中|Match en cours|मैच जारी है",
   L"Pantalla no reconocida|Unrecognized screen|未認識の画面|未识别的画面|Écran non reconnu|स्क्रीन पहचानी नहीं गई",
   L"Marcador final|Final score|最終スコア|最终比分|Score final|अंतिम स्कोर",
   L"Recompensas|Rewards|報酬|奖励|Récompenses|पुरस्कार",
   L"Revancha|Rematch|再戦|再战|Revanche|दोबारा मैच",
   L"Experiencia / resultados|Experience / results|経験値 / 結果|经验 / 结果|Expérience / résultats|अनुभव / परिणाम",
   L"Lista de equipos|Team list|チーム一覧|队伍列表|Liste des équipes|टीम सूची",
   L"Mapa de Crónica|Chronicle map|クロニクルマップ|编年史地图|Carte Chronique|क्रॉनिकल नक्शा",
   L"Descanso|Interval|ハーフタイム|中场休息|Mi-temps|मध्यांतर",
   L"Menú de ruta|Route menu|ルートメニュー|路线菜单|Menu de route|रूट मेनू",
   L"Preparación del partido|Match preparation|試合準備|赛前准备|Préparation du match|मैच की तैयारी",
   L"Presentación|Presentation|紹介|开场|Présentation|परिचय",
   L"Presentación de equipos|Team presentation|チーム紹介|队伍介绍|Présentation des équipes|टीम परिचय",
   L"Saque de centro|Kick-off|キックオフ|开球|Coup d’envoi|किक-ऑफ़",
   L"Alineación · Terminar edición|Lineup · Finish editing|編成 · 編集を終了|阵容 · 结束编辑|Composition · Terminer|लाइनअप · संपादन समाप्त करें",
   L"Estación / rutas|Station / routes|駅 / ルート|车站 / 路线|Gare / routes|स्टेशन / रूट",
   L"Sesión completada|Session complete|セッション完了|会话完成|Session terminée|सत्र पूर्ण",
   L"Jugando|Playing|プレイ中|正在比赛|En jeu|खेल जारी है",
   L"Esperando|Waiting|待機中|等待中|En attente|प्रतीक्षा",
   L"Pausado|Paused|一時停止|已暂停|En pause|रुका हुआ",
   L"Marcador leído|Score recognized|スコア認識済み|已识别比分|Score reconnu|स्कोर पहचाना गया",
   L"Resultado pendiente de revisión; continúa el farmeo|Result pending review; farming continues|結果は未確認です。周回を続けます|结果待确认；继续刷取|Résultat à vérifier ; les matchs continuent|परिणाम की समीक्षा लंबित है; फ़ार्मिंग जारी है",
   L"Marcador ilegible: resultado pendiente guardado; continúa el farmeo|Unreadable score: pending result saved; farming continues|スコアを読めません。未確認として保存し、周回を続けます|比分无法识别：已保存为待确认；继续刷取|Score illisible : résultat à vérifier enregistré ; les matchs continuent|स्कोर पढ़ा नहीं जा सका: लंबित परिणाम सहेजा गया; फ़ार्मिंग जारी है",
   L"Debug · Solo observación, sin pulsaciones|Diagnostics · Observation only, no inputs|診断 · 観察のみ、操作なし|诊断 · 仅观察，无输入|Diagnostic · Observation uniquement|निदान · केवल निरीक्षण, कोई इनपुट नहीं",
   L"Primero utiliza Observar juego, en Diagnóstico.|Use Observe game in Diagnostics first.|先に診断の「ゲームを確認」を使用してください。|请先使用诊断中的“观察游戏”。|Utilisez d’abord Observer le jeu dans Diagnostic.|पहले निदान में खेल देखें चुनें।",
   L"Detén la sesión actual antes de iniciar otra.|Stop the current session before starting another.|新しいセッションの前に現在のセッションを停止してください。|开始新会话前请停止当前会话。|Arrêtez la session actuelle avant d’en démarrer une autre.|नया सत्र शुरू करने से पहले वर्तमान सत्र समाप्त करें।",
   L"Revisa rival, dificultad, cantidad (1–100000) y aparición (1–20).|Check opponent, difficulty, count (1–100000) and occurrence (1–20).|相手、難易度、回数（1～100000）、順番（1～20）を確認してください。|请检查对手、难度、次数（1–100000）和序号（1–20）。|Vérifiez le rival, la difficulté, le nombre (1–100000) et l’occurrence (1–20).|प्रतिद्वंद्वी, कठिनाई, संख्या (1–100000) और क्रम (1–20) जाँचें।",
   L"El juego perdió el foco. Pulsa Reanudar.|The game lost focus. Press Resume.|ゲームが非アクティブになりました。「再開」を押してください。|游戏失去焦点，请点击继续。|Le jeu a perdu le focus. Cliquez sur Reprendre.|खेल का फ़ोकस हट गया। जारी रखें दबाएँ।",
   L"F8 está ocupado por otra aplicación; utiliza Detener.|F8 is in use by another app; use Stop.|F8 は他のアプリが使用中です。「停止」を使用してください。|F8 已被其他应用占用，请使用停止按钮。|F8 est utilisé ailleurs ; utilisez Arrêter.|F8 अन्य ऐप में उपयोग हो रहा है; समाप्त बटन दबाएँ।",
   L"Diagnóstico guardado en|Diagnostics saved to|診断の保存先|诊断已保存至|Diagnostic enregistré dans|निदान यहाँ सहेजा गया",
  };

  for(auto row:rows){std::array<std::wstring,6> a;std::wistringstream cells(row);for(auto& c:a)std::getline(cells,c,L'|');out.emplace(a[0],a);}return out;
 }();return data;
}
inline std::wstring tr(const std::wstring& s){
 if(language<1||language>5)return s;
 const auto& c=catalog();if(auto i=c.find(s);i!=c.end())return i->second[language];
 for(const auto& [key,v]:c)if(s.starts_with(key+L" · ")||s.starts_with(key+L": "))return v[language]+s.substr(key.size(),3)+tr(s.substr(key.size()+3));
 return s; // Game text, proper names, OCR evidence and numeric values stay unchanged.
}
inline std::wstring help(){
 static const wchar_t* texts[]={
 L"01  PREPARA EL JUEGO\r\n\r\nEl juego debe estar en español, con teclado y ratón. Abre una ruta de Crónica. La referencia visual es 1920 × 1080.\r\n\r\n02  CONFIGURA LA SESIÓN\r\n\r\nElige categoría, rival, dificultad y cantidad. Aparición distingue rivales repetidos. Sin límite repite hasta que lo detengas.\r\n\r\n03  INICIA Y CONTROLA\r\n\r\nZzen activa Comandante y usa Revancha. Mantén el juego visible; cambiar de ventana pausa la sesión. F8 detiene.\r\n\r\nDIAGNÓSTICO\r\n\r\nObservar juego no envía entradas. Puedes guardar capturas y registros. Los nombres y el texto leído del juego permanecen en español.\r\n\r\nGratis conserva los resultados solo durante la sesión. Premium añade seguimiento de recompensas.",
 L"01  PREPARE THE GAME\r\n\r\nSet the game language to Spanish and use keyboard/mouse controls. Open a Chronicle route. The visual reference is 1920 × 1080.\r\n\r\n02  SET UP YOUR SESSION\r\n\r\nChoose category, opponent, difficulty and count. Occurrence distinguishes repeated opponents. Unlimited repeats until stopped.\r\n\r\n03  START AND CONTROL\r\n\r\nZzen enables Commander and uses Rematch. Keep the game visible; changing windows pauses the session. F8 stops it.\r\n\r\nDIAGNOSTICS\r\n\r\nObserve game sends no inputs. Save screenshots and logs when needed. Names and recognized game text remain in Spanish.\r\n\r\nFree keeps results for the current session. Premium adds reward tracking.",
 L"01  ゲームの準備\r\n\r\nゲームの言語をスペイン語に設定し、キーボードとマウスを使用してください。クロニクルのルートを開きます。基準解像度は 1920 × 1080 です。\r\n\r\n02  セッション設定\r\n\r\nカテゴリー、相手、難易度、試合数を選択します。同名チームは順番で区別します。無制限は停止するまで続きます。\r\n\r\n03  開始と操作\r\n\r\nZzen はコマンダーを有効にし、再戦を選びます。ゲームを表示したままにしてください。別のウィンドウに切り替えると一時停止します。F8 で停止します。\r\n\r\n診断\r\n\r\n観察モードでは操作しません。画像やログを保存できます。ゲーム内の名前と認識した文字はスペイン語のままです。\r\n\r\n無料版の結果は現在のセッションのみ保存します。Premium は報酬の追跡を追加します。",
 L"01  准备游戏\r\n\r\n将游戏语言设置为西班牙语，并使用键盘和鼠标。在编年史模式打开一条路线。参考分辨率为 1920 × 1080。\r\n\r\n02  设置会话\r\n\r\n选择类别、对手、难度和场次。序号用于区分同名对手。不限次数将持续到手动停止。\r\n\r\n03  开始与控制\r\n\r\nZzen 会启用指挥官模式并选择再战。保持游戏可见，切换窗口会暂停会话。F8 停止。\r\n\r\n诊断\r\n\r\n观察模式不发送输入，可保存截图和日志。游戏名称和识别文字保持西班牙语。\r\n\r\n免费版仅保留当前会话结果。Premium 增加奖励追踪。",
 L"01  PRÉPARER LE JEU\r\n\r\nRéglez le jeu en espagnol et utilisez le clavier et la souris. Ouvrez une route de Chronique. Résolution de référence : 1920 × 1080.\r\n\r\n02  CONFIGURER LA SESSION\r\n\r\nChoisissez la catégorie, le rival, la difficulté et le nombre. L’occurrence distingue les rivaux répétés. Illimité continue jusqu’à l’arrêt.\r\n\r\n03  DÉMARRER ET CONTRÔLER\r\n\r\nZzen active Commandant et utilise Revanche. Gardez le jeu visible ; changer de fenêtre met la session en pause. F8 l’arrête.\r\n\r\nDIAGNOSTIC\r\n\r\nObserver le jeu n’envoie aucune commande. Les captures et journaux peuvent être enregistrés. Les noms et le texte du jeu restent en espagnol.\r\n\r\nLa version gratuite conserve les résultats de la session. Premium ajoute le suivi des récompenses.",
 L"01  खेल तैयार करें\r\n\r\nखेल की भाषा स्पेनिश रखें और कीबोर्ड व माउस का उपयोग करें। क्रॉनिकल में एक रूट खोलें। संदर्भ रिज़ॉल्यूशन 1920 × 1080 है।\r\n\r\n02  सत्र सेट करें\r\n\r\nश्रेणी, प्रतिद्वंद्वी, कठिनाई और संख्या चुनें। क्रम एक जैसे नाम वाले प्रतिद्वंद्वियों को अलग करता है। असीमित मोड आपके रोकने तक चलता है।\r\n\r\n03  शुरू करें और नियंत्रण रखें\r\n\r\nZzen कमांडर मोड चालू करके दोबारा मैच चुनता है। खेल को सामने रखें; विंडो बदलने पर सत्र रुकता है। F8 से सत्र समाप्त करें।\r\n\r\nनिदान\r\n\r\nखेल देखें विकल्प कोई इनपुट नहीं भेजता। स्क्रीनशॉट और लॉग सहेज सकते हैं। खेल के नाम और पहचाना गया पाठ स्पेनिश में रहते हैं।\r\n\r\nमुफ़्त संस्करण केवल वर्तमान सत्र के परिणाम रखता है। Premium पुरस्कारों का हिसाब जोड़ता है।"};
 return texts[language>=0&&language<6?language:0];
}
}
