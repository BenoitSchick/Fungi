#import "@preview/latexlike-report:1.0.0": *

#show: latexlike-report.with(
 
 // ======== Cover ============
 //Use content [] or none, except in author.
  author: "Benoit Schick", // must be a string ("")
  title: [Impedance measurement with an Impedance Analyzer 4294A],
  subtitle: [FUNGI Project],
  
  //participants: [Some other author], // In case of several authors (the name in author parameter will go first) Use content [] or none.
  
  affiliation: [Haute école d'Ingénieur de Fribourg (HEIA-FR)],
  year: [2026],
  //class: [Your class],
  other: [
  #v(1cm)

//  = Document Version History

  #table(
    columns: (1fr, 1fr, 2fr, 1fr),
    align: center,
    stroke: 0.5pt,
    [*Version*], [*Date*], [*Description*], [*Author*],

    [1.0], [2026-02-23], [Initial version], [Benoit Schick],
  )
],

  date: [#datetime.today().display()], // You could use #datetime.today().display() for the date.

  logo : image("Images/heia_logo.jpg"),

  //==========Theme ===============
  theme-color: rgb("#0f2787"),
  lang: "en", 
   participants-supplement: "Authors:", //Change it if you change the language
 
            
  //=========Font =================
  title-font: "New Computer Modern",
  font: "New Computer Modern",
  font-size : 11pt,
  font-weight: 400,

  //============ Math =============

  math-font: "New Computer Modern Math",
  math-weight: 400,
  math-ref-supplement: auto, //Use none for no supplement, auto for language based or any other function or string you like
  math-numbering: "(1.1)", // The numbering style you like
  
  // ---- Equate package ---
  // For more information, you can refer to equate documentation
  
  math-number-mode: "label", //Can be "label" or "line" 
  math-sub-numbering: true,  // true or false

  //===========Page style===============
  pagebreak-section: false, //For pagebreak after adding a new level one heading (=)
  show-outline:true, //true or false 
  page-paper:"a4",

  //-----chic header package----
  // customize the left/center/right header and left/center/right footer
  // you can add images, text, the number of the current page, etc, or put none if you don't want some part of the header or footer.
  //some usefull function: chic-page-number(), chic-heading-name()
  
  h-l : [#smallcaps[Projet FUNGI]],
  h-r :[#image("Images/heia_logo.jpg",width: 35%)],
  h-c : none,

  f-l : [],
  f-r : [],
  f-c : chic-page-number(),
  //=======================================
  //For more customitation you can check the documentation. !! Enjoy :D !!
)
 
//#show figure.caption: set align(left)
///////////////////////////Document starts/////////////////////////////

//#block(sticky: true)[
= Introduction
This report aims to analyze the electrical behavior of three bags containing respectively: substrate only, substrate inoculated with mycelium in the colonization phase, and fully colonized substrate. To this end, impedance measurements were performed using the Agilent 4294A impedance analyzer. The evolution of the magnitude, phase, resistance, and capacitance of the medium was investigated through a frequency sweep, with electrodes placed at various locations on each bag.
#set par(
  first-line-indent: 0em,
)
The working hypothesis is that the medium can be modeled as an equivalent RC system composed of:
  - Rs, representing the resistance of the medium, mainly related to its moisture content and electrical conductivity;
  - Cs, representing the capacitance of the system, influenced by the insulating material (plastic) between the electrodes, as well as the air gap between the substrate and the plastic.
#set par(
  first-line-indent: 0em,
)
A decrease in resistance is expected as the mycelium develops, due to its intrinsic electrical conductivity. In contrast, the capacitance should remain relatively constant, since the environmental factors affecting it vary only slightly. Consequently, the impedance magnitude and phase as functions of frequency are expected to exhibit the characteristic behavior of an RC circuit, similar to the graph shown below.

 #figure(
  caption: "Frequency response of an ideal RC system",
  image("Images/RC_ideal.png",width: 45%)
)<ideal_RC>

= Objectives 
The objectives of this study are:

  + *Characterize the substrate as an RC system and determine its resonance frequency.* This frequency represents the optimal point to detect the presence mycelium, where small variations in resistance or capacitance are easier to detect. 
  
  + *Observe changes in the electrical properties of the substrate during mycelium development.*
    By comparing the impedance, phase, resistance ($R_s$) and capacitance ($C_s$) before and after colonization, we aim to quantify the effect of mycelium growth on the electrical properties of the system.
//]
= Materials and Methods

== Measured sample
#grid(
  columns: (1fr, 1fr, 1fr),
  gutter: 0.5cm,

  grid.cell(
    figure(
      image("Images/sac_subonly.jpeg", width: 100%),
      caption: "Bag with substrate only",
    )
  ),

  grid.cell(
    figure(
      image("Images/sac_avantColo.jpeg", width: 100%),
      caption: "Bag with mycelium in the colonization phase",
    )
  ),

  grid.cell(
    figure(
      image("Images/sac_apresColo.jpeg", width: 100%),
      caption: "Bag with a fully colonized substrate",
    )
  ),
)

== Material used

#figure(
table(columns: 3, align: center,
  [*Equipment name*], [*Model/Reference*], [*Manufacturer*],
  [Precision Impedance Analyzer], [4294A], [Agilent],
  [ECG electrodes], [ECG-electrodes disposable type F-55], [Megro],
  [Metal rod electrodes (5cm)],
), caption: [Equipment and Instruments])

The impedance analyzer was configured to measure the magnitude, phase, capacitance ($C_s$), and resistance ($R_s$) over a frequency range of 40 Hz to 1 MHz. Measurements were performed using a two-terminal configuration, as illustrated in the figure below.
#figure(
  caption: "Two-terminal configuration",
  image("Images/2T.png", width:45%)
)

#set enum(indent: 20pt)
== Experimental procedure <sec_exp>
For each bag (substrate only, substrate with mycelium in the colonization phase and with fully colonized substrate), four measurements were performed using both ECG electrodes and metal rod electrodes. The procedure for each measurement was as follows :
      + *Electrode placement:* For each measurement, the electrodes were placed at a specific location on the bag (see @loc_meas) to capture spatial variations in the electrical properties. The ECG electrodes are attached to the plastic surface of the bag, whereas the metal rod electrodes penetrate the bag to establish direct contact with the substrate.
      + *Data acquisition:* The Impedance Analyzer performed a frequency sweep from 40Hz to 1MHz and at each frequency point, the magnitude, phase, resistance ($R_s$) and capacitance ($C_s$) were recorded.

#figure(
  caption: "Position of the H electrode (signal-injecting electrode) and L electrode (voltage-sensing electrode) on the bag for all four measurements",
  image("Images/location_measurements.png", width:45%)
) <loc_meas>

#set par(
  first-line-indent: 0em,
)
Step 1-2 were repeated for all four measurement to assess any variability in electrode placement. 

== Measurement Setup
#figure(
  caption: "Picture of the measurement setup",
  image("Images/setup_mesure.jpg", width:45%)
)
// == Mesures avec les électrodes en patch
// == Mesures avec les électrodes en tiges métalliques

= Results


== Impact of electrode selection on electrical properties
#figure(
  grid(
    columns: (1fr, 1fr),
    gutter: 0.5cm,

    grid.cell(
      image("Images/ecg_magPhase.png", width: 100%),
    ),

    grid.cell(
      image("Images/metal_magPhase.png", width: 100%),
    ),
 ),
  caption: "Impedance as a function of frequency using ECG electrodes (left) and metal rod electrodes (right)"
)<diff_ecg_metal>

The first observation is that the electrical response change significantly depending on whether ECG electrodes attached to the plastic bag or metal rod electrodes inserted directly into the substrate are used, as shown in @diff_ecg_metal. \

#set par(
  first-line-indent: 0em,
)

When using ECG electrodes, the impedance magnitude decreases as the frequency increases, while the phase remains approximately constant around -90°, with unstable behavior below 1kHz. This indicates that, with this measurement configuration, the system bahaves like an imperfect capacitor. Consequently, using ECG electrodes may not allow clear observation of mycelium growth, since the evolution of the resistive component, associated with the electrical conductivity of the mycelium, may not be properly distinguished.\

#set par(
  first-line-indent: 0em,
)

In contrast, when using metal rod electrodes, the system behaves more like an RC circuit (see @ideal_RC) : the impedance magnitude stabilizes at higher frequencies, where the resistive component becomes dominant over the capacitive component. This is further confirmed by the phase, which approaches 0° at high frequencies. Although this measurement method is more invasive, it provides better access to the resistive contribution of the medium and is therefore more suitable for monitoring the development of the mycelium within the substrate.\
With this type of electrode, the frequency range should not exceed 1MHz. Beyond this value the impedance begins to exhibit inductive behavior, mainly due to parasitic effects from measurement cables and electrodes. These parasitic inductances affect the measured impedance, making the results less representative of the substrate itself.

#set par(
  first-line-indent: 0em,
)

The next step is to analyze how the impedance varies with electrode placement and to compare the impedance responses of the three different bags.

== Measurements with ECG Electrodes
#figure(
  grid(
    columns: (1fr, 1fr, 1fr),
    gutter: 0.5cm,

    grid.cell(
      image("Images/err_ecgZ_sans.png", width: 100%),
    ),

    grid.cell(
      image("Images/err_ecgZ_avant.png", width: 100%),
    ),

    grid.cell(
      image("Images/err_ecgZ_apres.png", width: 100%),
    ),
  ),
  caption: "Relative error of impedance magnitude measured on the three bags at different positions using ECG electrodes"
)<ecg_3Z>

The results shown in @ecg_3Z compare the impedance magnitude measured at different electrode positions on each bag (as described in @sec_exp) in order to evaluate the spatial variability of the medium. \

#set par(
  first-line-indent: 0em,
)

The largest variation is observed for the bag containing only substrate, with differences exceeding 80% at low frequencies (between the H8L2 electrode configuration and the H5L11 configuation for example). This indicates that the electrical properties vary significantly across the substrate when no mycelium is present. Such variability may be attributed to non-uniform moisture distribution, air pockets... \
When mycelium is present in the bag, the differences between measurement positions are significantly reduced. This suggests that the presence of mycelium tends to homogenize the electrical properties of the medium : as the mycelium develops throughout the substrate, it improves electrical conductivity, resulting in a more uniform impedance across different locations. \
However, when the mycelium is fully developped, the variability slightly increases again compared to the intermediate state. //This may indicate structural changes in the substrate after complete colonization. \

#set par(
  first-line-indent: 0em,
)

// This observation may appear to contradict the assumption made in the previous section, where it was suggested that ECG electrodes may not reliably distinguish mycelium growth. Indeed, a clear distinction is observed when mycelium is inoculated into the substrate.

#grid(
  columns: (1fr, 1fr),
  gutter: 0.5cm,

  grid.cell(
    figure(
      image("Images/ecg_H8L2config_RC.png", width: 100%),
      caption: "Resistance and Capacitance measurements using ECG electrodes in H8L2 configuration"
    )
  ),

  grid.cell(
    figure(
      image("Images/ecg_H5L11config_RC.png", width: 100%),
      caption: "Resistance and Capacitance measurements using ECG electrodes in H5L11 configuration" 
    )
  ),
)
If we now compare the resistance ($R_s$) and capacitance ($C_s$) of the medium for the three bags while keeping the same electrode configuration, an unexpected bahavior can be observed : the resistive part of the medium appears higher when mycelium is present.\

#set par(
  first-line-indent: 0em,
)

This result is surprising, as we initially expected the resistance to decrease with mycelium growth due to its electrical conductivity. This suggests that, when using ECG electrodes attached to the plastic surface, the measured resistance does not only represent the intrinsic conductivity of the substrate.\ //Instead, it is likely influenced by contact effects rather than the true electrical properties of the substrate. \
Regarding the capacitance, the values remains relatively constant across the frequency range for the three bags, which is consistent with the hypothesis that the capacitive component mainly depends on the plastic layer. \

#set par(
  first-line-indent: 0em,
)

A final observation from these two figures is that the electrode configuration has a stronger impact on the resistive component than on the capacitive component of the medium.\
This difference can be explained as follows : the capacitance is mainly determined by the dielectric properties of the plastic layer which remains essentially unchanged between the two configurations. In constast the resistance depends on the current path through the substrate. When no mycelium is present, the resistive component increases as the current path length increases (with differences on the order of 1M$Omega$). When mycelium is present, the resistive component remains relatively constant despite changes in configuration. This behavior is consistent with the observations in @ecg_3Z, where the impedance appears more uniform.


// #figure(
//   grid(
//     columns: (1fr, 1fr),
//     gutter: 0.5cm,

//     grid.cell(
//       figure(
//         image("Images/ecg_subonly_RC.png", width: 100%),
//         caption: "Resistance and Capacitance measurements at different positions in substrate without mycelium using ECG electrodes"
//       )
//     ),

//     grid.cell(
//       figure(
//         image("Images/ecg_avantColo_RC.png", width: 100%),
//         caption: "Resistance and Capacitance measurements at different positions in partially colonized substrate using ECG electrodes"
//       )
//     ),
//   ),
// )

// #figure(
//   image("Images/ecg_apresColo_RC.png", width: 55%),
//   caption: "Resistance and Capacitance measurements at different positions in fully colonized substrate using ECG electrodes"
// )

== Measurements with Metal Rod Electrodes

#figure(
  grid(
    columns: (1fr, 1fr),
    gutter: 0.5cm,

    grid.cell(
      image("Images/err_metalZ_sans.png", width: 100%),
    ),

    grid.cell(
      image("Images/err_metalZ_apres.png", width: 100%),
    ),
  ),
  caption: "Relative error of impedance magnitude measured on the two bags (substrate only and fully colonized) at different positions using metal rod electrodes"
)<metal_3Z>

When the bags are penetrated with metal rod electrodes, the medium baheves like an RC circuit, as shown in @diff_ecg_metal. \

#set par(
  first-line-indent: 0em,
)

However, although we expected to observe a clear distinction between the substrate-only bag and the substrate inoculated with mycelium, this difference is not clearly confirmed in the measurements presented in @metal_3Z.\
At low frequencies, where the capacitive component dominates, the difference between electrode configurations (H8L5-H8L11 and H8L2) is approximately 30% for both the substrate only and the inoculated substrate. As the frequency increases and the resistive component becomes dominant, this difference decreases to around 10%. This trends suggests that the conductivity remains relatively stable across both electrode configurations and between the two bags.\

#set par(
  first-line-indent: 0em,
)

Therefore, the results indicate that the growth of the mycelium cannot be clearly detected through variations in the substrate conductivity alone.

#figure(
  grid(
    columns: (1fr, 1fr),
    gutter: 0.5cm,

    grid.cell(
      image("Images/metal_H8L2config_RC.png", width: 100%),
    ),

    grid.cell(
      image("Images/metal_H5L11config_RC.png", width: 100%),
    ),
 ),
caption: "Resistance and Capacitance measurements using metal rod electrodes in H8L2 (left) and H5L11 (right) configuration",    
)<metal_configRC>

If we look at the @metal_configRC, we observe the same overall behavior as with the ECG electrodes, but without the irregular behavior at low frequencies. However, the absolute values differ significantly: the measured capacitance is much lower (in the $mu$F range instead of nF), and the resistance is much lower (in the k$Omega$ range instead of M$Omega$). This difference is due to the fact that the current flows directrly through the substrate, without passing through the plastic layer of the bag.\ 

#set par(
  first-line-indent: 0em,
)
Across all electrode configurations, the capacitive component remains approximately constant over the frequency range. Regarding the resistive component, the measured resistance is higher in the H5L11 configuation than in H8L2 because of the longer effective current path. As with the ECG electrodes, we observe in the H8L2 configuration that the resistance of the inoculated substrate is approximately 30% higher than that of the substrate only at low frequencies. This difference decreases to about 10% at high frequencies, where the resistive contribution becomes dominant. \

#set par(
  first-line-indent: 0em,
)

However, this trend changes in the H5L11 configuration, where the resistance of the substrate-only bag becomes higher than that of the inoculated substrate.

= Conclusion

The objective of this study was to characterize the electrical behavior of three substrate bags (substrate only, inoculated substrate, and fully colonized substrate) and to evaluate whether impedance measurements could be used to monitor mycelium development. The working hypothesis was that the system could be modeled as an equivalent RC circuit, with the resistance related to the electrical conductivity of the medium and the capacitance mainly influenced by the plastic layer. A decrease in resistance was expected as the mycelium developed. \

#set par(
  first-line-indent: 0em,
)
The first measurements were performed using ECG electrodes attached to the plastic surface of the three bags. Under this configuration, the system behaved mainly as a capacitive element and the resistive contribution of the substrate could not be clearly isolated. The measurements may be influenced by the plastic layer and contact effects, making it difficult to detect changes related to the mycelium growth.\

#set par(
  first-line-indent: 0em,
)

In contrast, when using metal rod electrodes inserted directly into the substrate, the electrical response was closer to that of an RC circuit. However, a clear decrease in resistance with mycelium development was not observed just like with ECG electrodes. This suggests that the conductivity of the substrate may be dominated by other factors such as moisture distribution. \

#set par(
  first-line-indent: 0em,
)

In conclusion, the impedance measurements using either ECG electrodes or metal rod electrodes in the current configuration do not provide a clear and reliable indicator of mycelium growth. Alternative measurements techniques may be required to increase the sensivity to biological changes within the substrate.
// = Unnumbered section <nonumber>

// == Unnumbered section in level two <nonumber>

// === Simple list <nonumber>

// #figure(
//   table(columns: 2)[A][B][C][D],
//   caption: [Tables caption go up, but you can change it \ at the start of the document],
// )
/*
 #figure(
  caption: "Down for figures",
  image("Images/InSTEC.svg",width: 30%)
)<ascas>
*/

// #figure(
// table(columns: 6, align: center,
//   table.cell(colspan: 2 )[Resistencia $X 1$], table.cell(colspan: 2 )[Resistencia $ X 2$], table.cell(colspan: 2,  )[Resistencia $X 3$], 
//   [Valor de $ R 2$], [Valor de Resistencia], [Valor de $R 2$], [Valor de Resistencia], [Valor de $R 2$], [Valor de Resistencia], 
//   [10], [512], [2010], [101495], [55000], [2777228], 
//   [10], [511], [2010], [101495], [55010], [2777733], 
//   [10], [511], [2010], [101500], [55003], [2777379], 
//   [10], [511], [2010], [101500], [55020], [2778238], 
//   [10], [511], [2010], [101500], [55000], [2777228], 
//   [10], [511], [2010], [101495], [55000], [2777228], 
//   [10], [512], [2010], [101500], [55005], [2777480], 
//   [10], [511], [2010], [101500], [54973], [2775864], 
//   [10], [511], [2010], [101500], [55007], [2777581], 
//   [10], [511], [2010], [101495], [55004], [2777430], 
// ), caption: [Table cells are not justify])
