/*-----------------------------------------------------------------------
 *
 * File Name: RngMedBias.c
 *
 * Authors: Krishnan, B.  Itoh, Y.
 *
 * Revision: $Id$
 *
 * History:   Created by Sintes May 21, 2003
 *            Modified...
 *
 *-----------------------------------------------------------------------
 */

/************************************ <lalVerbatim file="RngMedBiasCV">
Author: Krishnan, B., Itoh, Y.
$Id$
************************************* </lalVerbatim> */

/* <lalLaTeX>  *******************************************************
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\section{MOdule \texttt{RngMedBias.c}}
\label{ss:RngMedBias.c}
Routine for finding bias in median for exponential distribution
To be used with any code which uses the running median to estimate PSD.  

For the exponential distribution with unit mean and variance, the value of the 
median is $\log(2.0)$ in the limit of infinite sample size. Thus, if we are 
using the running median to estimate the PSD, there is a correction factor 
of $\log(2.0)$.  However, for finite sample sizes (i.e. for finite block size 
values), there is a bias in the estimator of the median and the correction 
factor is different.  This program returns the correct normalization factor 
for block sizes from 1 to 1000.  For larger values it returns $\log(2.0)$ and 
returns and error for smaller values.  


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\subsubsection*{Prototypes}
\vspace{0.1in}
\input{RngMedBiasD}
\index{\verb&LALRngMedBias()&}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\subsubsection*{Description}



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\subsubsection*{Uses}
\begin{verbatim}
LALHO()
\end{verbatim}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\subsubsection*{Notes}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\vfill{\footnotesize\input{RngMedBiasCV}}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

*********************************************** </lalLaTeX> */


#include <lal/RngMedBias.h>

NRCSID (RNGMEDBIASC, "$Id$");

/*
 * The functions that make up the guts of this module
 */


/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */

/* >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/* *******************************  <lalVerbatim file="RngMedBiasD"> */
void LALRngMedBias (LALStatus   *status,
		 REAL8       *biasFactor,
		 INT4        blkSize
                 )
{/*   *********************************************  </lalVerbatim> */
  /* function to output corrcet bias factor for running median for given
     block size upto 500 */
  /* just reads from file with table of normalization factors */
  
  REAL8 bias[] = {
  1.0,1.0,
  0.83333333333333333333,
  0.83333333333333333333,
  0.78333333333333333333,
  0.78333333333333333333,
  0.75952380952380952381,
  0.75952380952380952381,
  0.74563492063492063492,
  0.74563492063492063492,
  0.73654401154401154401,
  0.73654401154401154401,
  0.73013375513375513376,
  0.73013375513375513376,
  0.72537185037185037185,
  0.72537185037185037185,
  0.72169537978361507773,
  0.72169537978361507773,
  0.71877140317542794323,
  0.71877140317542794323,
  0.71639045079447556228,
  0.71639045079447556228,
  0.71441416620949532512,
  0.71441416620949532512,
  0.71274749954282865846,
  0.71274749954282865846,
  0.71132299811832723396,
  0.71132299811832723396,
  0.71009147102473117484,
  0.71009147102473117484,
  0.70901620220752687377,
  0.70901620220752687377,
  0.7080692325105571768,
  0.7080692325105571768,
  0.70722889637610339528,
  0.70722889637610339528,
  0.70647814562535264453,
  0.70647814562535264453,
  0.70580338179269407503,
  0.70580338179269407503,
  0.70519362569513309942,
  0.70519362569513309942,
  0.70463991583909766199,
  0.70463991583909766199,
  0.70413486533404715694,
  0.70413486533404715694,
  0.70367233064394539931,
  0.70367233064394539931,
  0.70324716057591818843,
  0.70324716057591818843,
  0.70285500371317309039,
  0.70285500371317309039,
  0.70249215901070574641,
  0.70249215901070574641,
  0.70215545867400540971,
  0.70215545867400540971,
  0.70184217546598535958,
  0.70184217546598535958,
  0.70154994869801341335,
  0.70154994869801341335,
  0.70127672465429756636,
  0.70127672465429756636,
  0.70102070826924892325,
  0.70102070826924892325,
  0.70078032365386430786,
  0.70078032365386430786,
  0.70055418163667751456,
  0.70055418163667751456,
  0.70034105290692474388,
  0.70034105290692474388,
  0.70013984566346397929,
  0.70013984566346397929,
  0.69994958691156139178,
  0.69994958691156139178,
  0.6997694067313812116,
  0.6997694067313812116,
  0.69959852498155209334,
  0.69959852498155209334,
  0.69943624000914053865,
  0.69943624000914053865,
  0.69928191902148621767,
  0.69928191902148621767,
  0.69913498984135104282,
  0.69913498984135104282,
  0.69899493381894207924,
  0.69899493381894207924,
  0.69886127971576111158,
  0.69886127971576111158,
  0.69873359840830452322,
  0.69873359840830452322,
  0.69861149828620440112,
  0.69861149828620440112,
  0.69849462124085610753,
  0.69849462124085610753,
  0.69838263915798936621,
  0.69838263915798936621,
  0.69827525084183816346,
  0.69827525084183816346,
  0.69817217931019520324,
  0.69817217931019520324,
  0.69807316940920510423,
  0.69807316940920510423,
  0.69797798570465532316,
  0.69797798570465532316,
  0.69788641061308023158,
  0.69788641061308023158,
  0.69779824274145265267,
  0.69779824274145265267,
  0.697713295408798898,
  0.697713295408798898,
  0.6976313953268988161,
  0.6976313953268988161,
  0.69755238142045128133,
  0.69755238142045128133,
  0.6974761037698029213,
  0.6974761037698029213,
  0.69740242266163905452,
  0.69740242266163905452,
  0.69733120773499042897,
  0.69733120773499042897,
  0.69726233721157445101,
  0.69726233721157445101,
  0.6971956972009120493,
  0.6971956972009120493,
  0.69713118107187979123,
  0.69713118107187979123,
  0.69706868888340335079,
  0.69706868888340335079,
  0.69700812686789947482,
  0.69700812686789947482,
  0.6969494069618513245,
  0.6969494069618513245,
  0.69689244637857495175,
  0.69689244637857495175,
  0.69683716721881818005,
  0.69683716721881818005,
  0.69678349611534029255,
  0.69678349611534029255,
  0.69673136390806263641,
  0.69673136390806263641,
  0.69668070534676577724,
  0.69668070534676577724,
  0.69663145881864600968,
  0.69663145881864600968,
  0.69658356609833949627,
  0.69658356609833949627,
  0.69653697211828171974,
  0.69653697211828171974,
  0.69649162475749811734,
  0.69649162475749811734,
  0.69644747464712284141,
  0.69644747464712284141,
  0.69640447499112008943,
  0.69640447499112008943,
  0.69636258140083940237,
  0.69636258140083940237,
  0.6963217517421753488,
  0.6963217517421753488,
  0.69628194599422534482,
  0.69628194599422534482,
  0.69624312611844894731,
  0.69624312611844894731,
  0.69620525593742948204,
  0.69620525593742948204,
  0.69616830102242578654,
  0.69616830102242578654,
  0.69613222858897942625,
  0.69613222858897942625,
  0.69609700739991208334,
  0.69609700739991208334,
  0.69606260767510988176,
  0.69606260767510988176,
  0.69602900100754703729,
  0.69602900100754703729,
  0.69599616028505114238,
  0.69599616028505114238,
  0.69596405961735725435,
  0.69596405961735725435,
  0.69593267426803831643,
  0.69593267426803831643,
  0.69590198059093579954,
  0.69590198059093579954,
  0.69587195597074724493,
  0.69587195597074724493,
  0.69584257876745699816,
  0.69584257876745699816,
  0.69581382826432319332,
  0.69581382826432319332,
  0.69578568461915827156,
  0.69578568461915827156,
  0.69575812881866226715,
  0.69575812881866226715,
  0.69573114263558800117,
  0.69573114263558800117,
  0.69570470858853539742,
  0.69570470858853539742,
  0.69567880990418856224,
  0.69567880990418856224,
  0.69565343048182421525,
  0.69565343048182421525,
  0.69562855485993366799,
  0.69562855485993366799,
  0.69560416818481295395,
  0.69560416818481295395,
  0.69558025618098703333,
  0.69558025618098703333,
  0.69555680512334433365,
  0.69555680512334433365,
  0.69553380181086733696,
  0.69553380181086733696,
  0.69551123354185357032,
  0.69551123354185357032,
  0.69548908809052927233,
  0.69548908809052927233,
  0.6954673536849652645,
  0.6954673536849652645,
  0.69544601898621121091,
  0.69544601898621121091,
  0.69542507306857055907,
  0.69542507306857055907,
  0.69540450540094406792,
  0.69540450540094406792,
  0.69538430582917498942,
  0.69538430582917498942,
  0.69536446455933371958,
  0.69536446455933371958,
  0.69534497214188410748,
  0.69534497214188410748,
  0.69532581945667764153,
  0.69532581945667764153,
  0.6953069976987254488,
  0.6953069976987254488,
  0.69528849836470147366,
  0.69528849836470147366,
  0.69527031324013337037,
  0.69527031324013337037,
  0.69525243438724057197,
  0.69525243438724057197,
  0.69523485413338170625,
  0.69523485413338170625,
  0.69521756506007603543,
  0.69521756506007603543,
  0.69520055999256591742,
  0.69520055999256591742,
  0.69518383198988943699,
  0.69518383198988943699,
  0.69516737433543434993,
  0.69516737433543434993,
  0.69515118052794633335,
  0.69515118052794633335,
  0.69513524427296625367,
  0.69513524427296625367,
  0.69511955947467275972,
  0.69511955947467275972,
  0.69510412022810799208,
  0.69510412022810799208,
  0.69508892081176557963,
  0.69508892081176557963,
  0.69507395568052137862,
  0.69507395568052137862,
  0.69505921945888860526,
  0.69505921945888860526,
  0.69504470693458012704,
  0.69504470693458012704,
  0.69503041305236171652,
  0.69503041305236171652,
  0.69501633290818104011,
  0.69501633290818104011,
  0.69500246174355805837,
  0.69500246174355805837,
  0.69498879494022335836,
  0.69498879494022335836,
  0.69497532801499172724,
  0.69497532801499172724,
  0.69496205661485901324,
  0.69496205661485901324,
  0.69494897651231100926,
  0.69494897651231100926,
  0.69493608360083373947,
  0.69493608360083373947,
  0.69492337389061513245,
  0.69492337389061513245,
  0.69491084350442863018,
  0.69491084350442863018,
  0.6948984886736898113,
  0.6948984886736898113,
  0.694886305734677604,
  0.694886305734677604,
  0.69487429112491212918,
  0.69487429112491212918,
  0.69486244137968165164,
  0.69486244137968165164,
  0.69485075312871152681,
  0.69485075312871152681,
  0.694839223092968416,
  0.694839223092968416,
  0.69482784808159340463,
  0.69482784808159340463,
  0.69481662498895799802,
  0.69481662498895799802,
  0.69480555079183728927,
  0.69480555079183728927,
  0.69479462254669489423,
  0.69479462254669489423,
  0.69478383738707453185,
  0.69478383738707453185,
  0.69477319252109339455,
  0.69477319252109339455,
  0.69476268522903270443,
  0.69476268522903270443,
  0.69475231286102108738,
  0.69475231286102108738,
  0.69474207283480662027,
  0.69474207283480662027,
  0.69473196263361361653,
  0.69473196263361361653,
  0.69472197980408041364,
  0.69472197980408041364,
  0.69471212195427461328,
  0.69471212195427461328,
  0.69470238675178240145,
  0.69470238675178240145,
  0.69469277192186874262,
  0.69469277192186874262,
  0.69468327524570539979,
  0.69468327524570539979,
  0.69467389455866388087,
  0.69467389455866388087,
  0.69466462774867055297,
  0.69466462774867055297,
  0.6946554727546212991,
  0.6946554727546212991,
  0.69464642756485321777,
  0.69464642756485321777,
  0.69463749021567098522,
  0.69463749021567098522,
  0.69462865878992561289,
  0.69462865878992561289,
  0.69461993141564343942,
  0.69461993141564343942,
  0.69461130626470329797,
  0.69461130626470329797,
  0.69460278155155989524,
  0.69460278155155989524,
  0.69459435553201152989,
  0.69459435553201152989,
  0.69458602650201036383,
  0.69458602650201036383,
  0.69457779279651354204,
  0.69457779279651354204,
  0.6945696527883735339,
  0.6945696527883735339,
  0.6945616048872661427,
  0.6945616048872661427,
  0.69455364753865470004,
  0.69455364753865470004,
  0.69454577922278902793,
  0.69454577922278902793,
  0.6945379984537378149,
  0.6945379984537378149,
  0.69453030377845311192,
  0.69453030377845311192,
  0.69452269377586571104,
  0.69452269377586571104,
  0.69451516705601022406,
  0.69451516705601022406,
  0.69450772225917872959,
  0.69450772225917872959,
  0.69450035805510190621,
  0.69450035805510190621,
  0.69449307314215661591,
  0.69449307314215661591,
  0.69448586624659894633,
  0.69448586624659894633,
  0.69447873612182176273,
  0.69447873612182176273,
  0.69447168154763586059,
  0.69447168154763586059,
  0.69446470132957384834,
  0.69446470132957384834,
  0.69445779429821592597,
  0.69445779429821592597,
  0.69445095930853676039,
  0.69445095930853676039,
  0.69444419523927269112,
  0.69444419523927269112,
  0.69443750099230853212,
  0.69443750099230853212,
  0.69443087549208326511,
  0.69443087549208326511,
  0.69442431768501394909,
  0.69442431768501394909,
  0.69441782653893719778,
  0.69441782653893719778,
  0.69441140104256760323,
  0.69441140104256760323,
  0.69440504020497250871,
  0.69440504020497250871,
  0.69439874305506255795,
  0.69439874305506255795,
  0.69439250864109747067,
  0.69439250864109747067,
  0.69438633603020651601,
  0.69438633603020651601,
  0.69438022430792317657,
  0.69438022430792317657,
  0.69437417257773351534,
  0.69437417257773351534,
  0.69436817996063777729,
  0.69436817996063777729,
  0.6943622455947247751,
  0.6943622455947247751,
  0.69435636863475862639,
  0.69435636863475862639,
  0.69435054825177742622,
  0.69435054825177742622,
  0.69434478363270345463,
  0.69434478363270345463,
  0.69433907397996453421,
  0.69433907397996453421,
  0.69433341851112616751,
  0.69433341851112616751,
  0.69432781645853409778,
  0.69432781645853409778,
  0.69432226706896695016,
  0.69432226706896695016,
  0.69431676960329862326,
  0.69431676960329862326,
  0.69431132333617011314,
  0.69431132333617011314,
  0.69430592755567046387,
  0.69430592755567046387,
  0.69430058156302654975,
  0.69430058156302654975,
  0.69429528467230140541,
  0.69429528467230140541,
  0.69429003621010083018,
  0.69429003621010083018,
  0.69428483551528800318,
  0.69428483551528800318,
  0.69427968193870585517,
  0.69427968193870585517,
  0.69427457484290695218,
  0.69427457484290695218,
  0.69426951360189065498,
  0.69426951360189065498,
  0.69426449760084732676,
  0.69426449760084732676,
  0.6942595262359093694,
  0.6942595262359093694,
  0.69425459891390887666,
  0.69425459891390887666,
  0.69424971505214170012,
  0.69424971505214170012,
  0.69424487407813773052,
  0.69424487407813773052,
  0.69424007542943720459,
  0.69424007542943720459,
  0.69423531855337285357,
  0.69423531855337285357,
  0.69423060290685771635,
  0.69423060290685771635,
  0.69422592795617844601,
  0.69422592795617844601,
  0.69422129317679394472,
  0.69422129317679394472,
  0.69421669805313916717,
  0.69421669805313916717,
  0.69421214207843393873,
  0.69421214207843393873,
  0.69420762475449663919,
  0.69420762475449663919,
  0.6942031455915626083,
  0.6942031455915626083,
  0.69419870410810713417,
  0.69419870410810713417,
  0.69419429983067289003,
  0.69419429983067289003,
  0.69418993229370168957,
  0.69418993229370168957,
  0.69418560103937043524,
  0.69418560103937043524,
  0.69418130561743113814,
  0.69418130561743113814,
  0.69417704558505489208,
  0.69417704558505489208,
  0.69417282050667968822,
  0.69417282050667968822,
  0.69416862995386196051,
  0.69416862995386196051,
  0.69416447350513175559,
  0.69416447350513175559,
  0.69416035074585142445,
  0.69416035074585142445,
  0.69415626126807773615,
  0.69415626126807773615,
  0.69415220467042731751,
  0.69415220467042731751,
  0.69414818055794532542,
  0.69414818055794532542,
  0.69414418854197726154,
  0.69414418854197726154,
  0.69414022824004384214,
  0.69414022824004384214,
  0.69413629927571883821,
  0.69413629927571883821,
  0.69413240127850980421,
  0.69413240127850980421,
  0.69412853388374161586,
  0.69412853388374161586,
  0.69412469673244274014,
  0.69412469673244274014,
  0.69412088947123416314,
  0.69412088947123416314,
  0.69411711175222090335,
  0.69411711175222090335,
  0.69411336323288604062,
  0.69411336323288604062,
  0.69410964357598719297,
  0.69410964357598719297,
  0.69410595244945537546,
  0.69410595244945537546,
  0.69410228952629617749,
  0.69410228952629617749,
  0.69409865448449319676,
  0.69409865448449319676,
  0.69409504700691366992,
  0.69409504700691366992,
  0.69409146678121624195,
  0.69409146678121624195,
  0.69408791349976081787,
  0.69408791349976081787,
  0.69408438685952044207,
  0.69408438685952044207,
  0.69408088656199515242,
  0.69408088656199515242,
  0.69407741231312775755,
  0.69407741231312775755,
  0.6940739638232214875,
  0.6940739638232214875,
  0.69407054080685946929,
  0.69407054080685946929,
  0.69406714298282598034,
  0.69406714298282598034,
  0.6940637700740294342,
  0.6940637700740294342,
  0.69406042180742705425,
  0.69406042180742705425,
  0.69405709791395119236,
  0.69405709791395119236,
  0.69405379812843725077,
  0.69405379812843725077,
  0.69405052218955316659,
  0.69405052218955316659,
  0.69404726983973041966,
  0.69404726983973041966,
  0.69404404082509652534,
  0.69404404082509652534,
  0.69404083489540897524,
  0.69404083489540897524,
  0.69403765180399058971,
  0.69403765180399058971,
  0.69403449130766624693,
  0.69403449130766624693,
  0.69403135316670095477,
  0.69403135316670095477,
  0.69402823714473923199,
  0.69402823714473923199,
  0.6940251430087457668,
  0.6940251430087457668,
  0.69402207052894732148,
  0.69402207052894732148,
  0.69401901947877585246,
  0.69401901947877585246,
  0.69401598963481281655,
  0.69401598963481281655,
  0.69401298077673463438,
  0.69401298077673463438,
  0.69400999268725928313,
  0.69400999268725928313,
  0.69400702515209399143,
  0.69400702515209399143,
  0.69400407795988400988,
  0.69400407795988400988,
  0.69400115090216243161,
  0.69400115090216243161,
  0.6939982437733010377,
  0.6939982437733010377,
  0.69399535637046214322,
  0.69399535637046214322,
  0.69399248849355142023,
  0.69399248849355142023,
  0.69398963994517167455,
  0.69398963994517167455,
  0.69398681053057755407,
  0.69398681053057755407,
  0.69398400005763116666,
  0.69398400005763116666,
  0.69398120833675858638,
  0.69398120833675858638,
  0.69397843518090722753,
  0.69397843518090722753,
  0.69397568040550406615,
  0.69397568040550406615,
  0.69397294382841468955,
  0.69397294382841468955,
  0.6939702252699031547,
  0.6939702252699031547,
  0.69396752455259263703,
  0.69396752455259263703,
  0.6939648415014268513,
  0.6939648415014268513,
  0.69396217594363222719,
  0.69396217594363222719,
  0.69395952770868082231,
  0.69395952770868082231,
  0.69395689662825395582,
  0.69395689662825395582,
  0.69395428253620654664,
  0.69395428253620654664,
  0.69395168526853214012,
  0.69395168526853214012,
  0.69394910466332860779,
  0.69394910466332860779,
  0.69394654056076450522,
  0.69394654056076450522,
  0.69394399280304607321,
  0.69394399280304607321,
  0.69394146123438486799,
  0.69394146123438486799,
  0.69393894570096600652,
  0.69393894570096600652,
  0.69393644605091701338,
  0.69393644605091701338,
  0.69393396213427725581,
  0.69393396213427725581,
  0.69393149380296795415,
  0.69393149380296795415,
  0.693929040910762755,
  0.693929040910762755,
  0.69392660331325885484,
  0.69392660331325885484,
  0.69392418086784866216,
  0.69392418086784866216,
  0.69392177343369198634,
  0.69392177343369198634,
  0.69391938087168874203,
  0.69391938087168874203,
  0.69391700304445215773,
  0.69391700304445215773,
  0.69391463981628247795,
  0.69391463981628247795,
  0.69391229105314114817,
  0.69391229105314114817,
  0.69390995662262547247,
  0.69390995662262547247,
  0.6939076363939437336,
  0.6939076363939437336,
  0.69390533023789076581,
  0.69390533023789076581,
  0.69390303802682397078,
  0.69390303802682397078,
  0.69390075963463976733,
  0.69390075963463976733,
  0.69389849493675046576,
  0.69389849493675046576,
  0.69389624381006155796,
  0.69389624381006155796,
  0.69389400613294941453,
  0.69389400613294941453,
  0.6938917817852393805,
  0.6938917817852393805,
  0.69388957064818426128,
  0.69388957064818426128,
  0.69388737260444319083,
  0.69388737260444319083,
  0.69388518753806087413,
  0.69388518753806087413,
  0.6938830153344471962,
  0.6938830153344471962,
  0.69388085588035719016,
  0.69388085588035719016,
  0.693878709063871357,
  0.693878709063871357,
  0.69387657477437632989,
  0.69387657477437632989,
  0.69387445290254587594,
  0.69387445290254587594,
  0.69387234334032222859,
  0.69387234334032222859,
  0.69387024598089774402,
  0.69387024598089774402,
  0.69386816071869687488,
  0.69386816071869687488,
  0.69386608744935845513,
  0.69386608744935845513,
  0.69386402606971828956,
  0.69386402606971828956,
  0.69386197647779204207,
  0.69386197647779204207,
  0.69385993857275841664,
  0.69385993857275841664,
  0.69385791225494262514,
  0.69385791225494262514,
  0.69385589742580013642,
  0.69385589742580013642,
  0.69385389398790070099,
  0.69385389398790070099,
  0.69385190184491264588,
  0.69385190184491264588,
  0.69384992090158743441,
  0.69384992090158743441,
  0.69384795106374448565,
  0.69384795106374448565,
  0.69384599223825624839,
  0.69384599223825624839,
  0.69384404433303352491,
  0.69384404433303352491,
  0.69384210725701103933,
  0.69384210725701103933,
  0.69384018092013324614,
  0.69384018092013324614,
  0.69383826523334037403,
  0.69383826523334037403,
  0.69383636010855470057,
  0.69383636010855470057,
  0.69383446545866705331,
  0.69383446545866705331,
  0.6938325811975235329,
  0.6938325811975235329,
  0.69383070723991245407,
  0.69383070723991245407,
  0.69382884350155150013,
  0.69382884350155150013,
  0.69382698989907508722,
  0.69382698989907508722,
  0.69382514635002193402,
  0.69382514635002193402,
  0.6938233127728228332,
  0.6938233127728228332,
  0.69382148908678862085,
  0.69382148908678862085,
  0.69381967521209833996,
  0.69381967521209833996,
  0.69381787106978759448,
  0.69381787106978759448,
  0.69381607658173709041,
  0.69381607658173709041,
  0.6938142916706613602,
  0.6938142916706613602,
  0.69381251626009766735,
  0.69381251626009766735,
  0.6938107502743950876,
  0.6938107502743950876,
  0.69380899363870376362,
  0.69380899363870376362,
  0.69380724627896432991,
  0.69380724627896432991,
  0.69380550812189750472,
  0.69380550812189750472,
  0.6938037790949938461,
  0.6938037790949938461,
  0.69380205912650366884,
  0.69380205912650366884,
  0.69380034814542711955,
  0.69380034814542711955,
  0.69379864608150440686,
  0.69379864608150440686,
  0.69379695286520618406,
  0.69379695286520618406,
  0.69379526842772408121,
  0.69379526842772408121,
  0.69379359270096138423,
  0.69379359270096138423,
  0.69379192561752385818,
  0.69379192561752385818,
  0.69379026711071071219,
  0.69379026711071071219,
  0.69378861711450570347,
  0.69378861711450570347,
  0.69378697556356837788,
  0.69378697556356837788,
  0.69378534239322544477,
  0.69378534239322544477,
  0.69378371753946228346,
  0.69378371753946228346,
  0.69378210093891457919,
  0.69378210093891457919,
  0.69378049252886008626,
  0.69378049252886008626,
  0.69377889224721051593,
  0.69377889224721051593,
  0.69377730003250354713,
  0.69377730003250354713,
  0.69377571582389495755,
  0.69377571582389495755,
  0.69377413956115087326,
  0.69377413956115087326,
  0.69377257118464013459,
  0.69377257118464013459,
  0.69377101063532677629,
  0.69377101063532677629,
  0.69376945785476262005,
  0.69376945785476262005,
  0.69376791278507997737,
  0.69376791278507997737,
  0.69376637536898446078,
  0.69376637536898446078,
  0.69376484554974790177,
  0.69376484554974790177,
  0.69376332327120137333,
  0.69376332327120137333,
  0.69376180847772831544,
  0.69376180847772831544,
  0.69376030111425776179,
  0.69376030111425776179,
  0.69375880112625766579,
  0.69375880112625766579,
  0.69375730845972832444,
  0.69375730845972832444,
  0.69375582306119589819,
  0.69375582306119589819,
  0.69375434487770602523,
  0.69375434487770602523,
  0.69375287385681752861,
  0.69375287385681752861,
  0.69375140994659621466,
  0.69375140994659621466,
  0.69374995309560876107,
  0.69374995309560876107,
  0.69374850325291669315,
  0.69374850325291669315,
  0.69374706036807044696,
  0.69374706036807044696,
  0.69374562439110351751,
  0.69374562439110351751,
  0.69374419527252669096,
  0.69374419527252669096,
  0.69374277296332235917,
  0.69374277296332235917,
  0.69374135741493891543,
  0.69374135741493891543,
  0.69373994857928522977,
  0.69373994857928522977,
  0.69373854640872520285,
  0.69373854640872520285,
  0.69373715085607239681,
  0.69373715085607239681,
  0.69373576187458474208,
  0.69373576187458474208,
  0.6937343794179593187,
  0.6937343794179593187,
  0.69373300344032721109,
  0.69373300344032721109,
  0.69373163389624843491,
  0.69373163389624843491,
  0.69373027074070693501,
  0.69373027074070693501,
  0.69372891392910565309,
  0.69372891392910565309,
  0.69372756341726166422,
  0.69372756341726166422,
  0.6937262191614013808,
  0.6937262191614013808,
  0.6937248811181558231,
  0.6937248811181558231,
  0.69372354924455595522,
  0.69372354924455595522,
  0.69372222349802808538,
  0.69372222349802808538,
  0.69372090383638932956,
  0.69372090383638932956,
  0.69371959021784313747,
  0.69371959021784313747,
  0.69371828260097487987,
  0.69371828260097487987,
  0.69371698094474749615,
  0.69371698094474749615,
  0.69371568520849720137,
  0.69371568520849720137,
  0.69371439535192925173,
  0.69371439535192925173,
  0.69371311133511376751,
  0.69371311133511376751,
  0.69371183311848161269,
  0.69371183311848161269,
  0.69371056066282033021,
  0.69371056066282033021,
  0.69370929392927013209,
  0.69370929392927013209,
  0.69370803287931994357,
  0.69370803287931994357,
  0.69370677747480350028,
  0.69370677747480350028,
  0.69370552767789549783,
  0.69370552767789549783,
  0.69370428345110779278,
  0.69370428345110779278,
  0.69370304475728565435,
  0.69370304475728565435,
  0.69370181155960406599,
  0.69370181155960406599,
  0.69370058382156407611,
  0.69370058382156407611,
  0.69369936150698919711,
  0.69369936150698919711,
  0.69369814458002185213,
  0.69369814458002185213,
  0.69369693300511986854,
  0.69369693300511986854,
  0.69369572674705301772,
  0.69369572674705301772,
  0.69369452577089960022,
  0.69369452577089960022,
  0.69369333004204307573,
  0.69369333004204307573,
  0.69369213952616873706,
  0.69369213952616873706,
  0.69369095418926042761,
  0.69369095418926042761,
  0.69368977399759730152,
  0.69368977399759730152,
  0.69368859891775062594,
  0.69368859891775062594,
  0.69368742891658062477,
  0.69368742891658062477,
  0.69368626396123336323,
  0.69368626396123336323,
  0.69368510401913767264,
  0.69368510401913767264,
  0.69368394905800211486,
  0.69368394905800211486,
  0.69368279904581198564,
  0.69368279904581198564,
  0.69368165395082635658,
  0.69368165395082635658,
  0.6936805137415751548,
  0.6936805137415751548,
  0.69367937838685627998,
  0.69367937838685627998,
  0.69367824785573275815,
  0.69367824785573275815,
  0.69367712211752993165,
  0.69367712211752993165,
  0.69367600114183268477,
  0.69367600114183268477,
  0.69367488489848270452,
  0.69367488489848270452,
  0.69367377335757577607,
  0.69367377335757577607,
  0.69367266648945911217,
  0.69367266648945911217,
  0.69367156426472871634,
  0.69367156426472871634,
  0.69367046665422677905,
  0.69367046665422677905,
  0.69366937362903910663,
  0.69366937362903910663,
  0.69366828516049258222,
  0.69366828516049258222,
  0.69366720122015265853,
  0.69366720122015265853,
  0.69366612177982088175,
  0.69366612177982088175,
  0.69366504681153244626,
  0.69366504681153244626,
  0.69366397628755377966,
  0.69366397628755377966,
  0.69366291018038015771,
  0.69366291018038015771,
  0.6936618484627333487,
  0.6936618484627333487,
  0.69366079110755928689,
  0.69366079110755928689,
  0.69365973808802577455,
  0.69365973808802577455,
  0.69365868937752021218,
  0.69365868937752021218,
  0.69365764494964735666,
  0.69365764494964735666,
  0.6936566047782271066,
  0.6936566047782271066,
  0.69365556883729231494,
  0.69365556883729231494,
  0.69365453710108662801,
  0.69365453710108662801,
  0.69365350954406235095,
  0.69365350954406235095,
  0.69365248614087833896,
  0.69365248614087833896,
  0.69365146686639791413,
  0.69365146686639791413,
  0.69365045169568680735,
  0.69365045169568680735,
  0.69364944060401112511,
  0.69364944060401112511,
  0.69364843356683534073,
  0.69364843356683534073,
  0.69364743055982030967,
  0.69364743055982030967  };


  /* --------------------------------------------- */
  INITSTATUS (status, "LALRngMedBias", RNGMEDBIASC);
  ATTATCHSTATUSPTR (status);   

  /* check arguments are not null and block size is positive*/
  ASSERT (biasFactor, status, RNGMEDBIASH_ENULL, RNGMEDBIASH_MSGENULL); 
  ASSERT (blkSize > 0, status,  RNGMEDBIASH_EVAL, RNGMEDBIASH_MSGEVAL);


  if( blkSize > 1000) { 
    /* if the Block_size=1000, return log(2.0) =  0.693147 */
    *biasFactor = log(2.0);
  }
  else {
    *biasFactor = bias[ blkSize - 1 ];
  }

  DETATCHSTATUSPTR (status);
  /* normal exit */
  RETURN (status);
}








