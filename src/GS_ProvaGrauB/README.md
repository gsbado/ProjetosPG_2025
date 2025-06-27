# Mochi's Journey – Atividade Final de Processamento Gráfico

Este diretório contém o projeto **Mochi's Journey**, desenvolvido como Atividade Final para a disciplina de Processamento Gráfico (Unisinos).

## 👥 Integrantes do Grupo

- Gabriela Bado e Sadi Júnior



## 📁 Estrutura

- `AtividadeFinalGB.cpp` - Código principal do jogo Mochi's Journey.
- `map.txt` - Arquivo de mapa utilizado pelo jogo.
- `assets/sprites/Mochi/Walk/Slime1_Walk_full.png` - Spritesheet de animação do Mochi.
- `assets/sprites/coin/coin.png` - Sprite da moeda dourada.
- `assets/tilesets/tilesetIso.png` - Tileset isométrico utilizado no mapa.
- `assets/sprites/Tree/Mega_tree1.png` - Sprite das árvores decorativas utilizadas no mapa.
- `assets/game-over.png` e `assets/winner.png` - Mensagens apresentadas na tela de Game Over e de Vitória.



## ⚙️ Como Executar

Para compilar e rodar este projeto, certifique-se de ter um compilador C++ e as bibliotecas necessárias instaladas (GLFW, GLAD, GLM, stb_image). Você pode usar o Visual Studio Code, CLion, ou outro editor/IDE de sua preferência.

1. Abra o terminal e entre na pasta `build` do projeto: `cd build`
2. Gere os arquivos de build com o CMake (ou configure seu projeto na IDE).
3. Compile o projeto (pode utilizar `cmake --build .` no terminal).
4. Execute o programa gerado (`./AtividadeFinalGB`).

Certifique-se de que as DLLs das bibliotecas estejam acessíveis no PATH do sistema, se necessário.

______________________________________________



## 💡 Sobre o Programa

- O jogo utiliza OpenGL moderno, shaders customizados e projeção ortográfica.
- O mapa é carregado de arquivo e renderizado em formato isométrico (diamond).
- O personagem Mochi é animado com spritesheet.
- O sistema de colisão, coleta de moedas e vidas é feito por meio de metadados dos tiles.
- O código é modular e pode ser expandido para incluir novos desafios, inimigos ou power-ups.



## 📖 História

Há muito tempo, em uma terra distante feita de caminhos incertos, vivia Mochi, uma criatura gelatinosa, doce e corajosa. Mochi não é um monstro nem um herói clássico – ele é um ser encantado nascido da fusão mágica entre gotas de orvalho e poeira de estrela, o que lhe deu forma, brilho e uma curiosidade infinita.

Mochi vive no Planeta Pixelândia, um mundo isométrico que guarda segredos antigos, desafios perigosos e recompensas escondidas.

Um dia, os ventos do Norte sussurraram uma profecia:

> “Apenas aquele que reunir as Cinco Moedas Douradas da Coragem poderá restaurar o equilíbrio entre os elementos e proteger a terra da corrupção pixelada.”

Determinado a provar seu valor e proteger sua terra, Mochi parte em uma jornada para reunir as moedas espalhadas no mapa. Mas o caminho não é fácil... Lava brota de fendas traiçoeiras e pode derretê-lo em segundos. Águas o assustam, pois Mochi, sendo leve e mágico, não sabe nadar. Apenas os caminhos seguros, como areia, grama e terra firme, podem levá-lo até seu destino.

Cada moeda representa uma virtude: coragem, sabedoria, equilíbrio, paciência e amor. Não há finais gloriosos, portais nem créditos rolando, o objetivo de Mochi é simples: explorar, sobreviver e encontrar todas as moedas. Só assim ele sentirá que cumpriu seu papel em Pixelândia.

Agora, Mochi conta com a sua ajuda. Guie-o com atenção pelos caminhos pixelados, reúna as cinco moedas da virtude e descubra se é possível salvar Pixelândia antes que ela se desfaça em blocos de esquecimento.

O destino de Mochi e Pixelândia está em suas mãos.


## 🚀 Regras do Jogo

- **Objetivo:** Colete as 5 moedas douradas do mapa para vencer. A mensagem de vitória será exibida automaticamente.
- **Moedas:** Ao passar sobre uma moeda, ela desaparece do tile. Quando todas as moedas forem coletadas, o jogo termina com vitória. Moedas já coletadas não desaparecem, mesmo após Mochi perder uma vida.
- **Vidas:** Mochi começa com 3 vidas. Ao perder uma vida, Mochi retorna à posição inicial do mapa. Se perder todas, aparece uma mensagem de **Game Over**.
- **Lava:** Se Mochi pisar em um tile de lava, perde uma vida.
- **Obstáculos:** Tiles de água não podem ser atravessados.
- **Limites do Mapa:** Mochi não pode sair dos limites do mapa de Pixelândia.


## 🎮 Controles

- **W** ou **Seta Cima**: Move o Mochi para cima (norte)
- **S** ou **Seta Baixo**: Move o Mochi para baixo (sul)
- **A** ou **Seta Esquerda**: Move o Mochi para a esquerda (oeste)
- **D** ou **Seta Direita**: Move o Mochi para a direita (leste)
- **Q**: Move o Mochi para a diagonal noroeste
- **E**: Move o Mochi para a diagonal nordeste
- **Z**: Move o Mochi para a diagonal sudoeste
- **C**: Move o Mochi para a diagonal sudeste
- **R**: Reinicia o jogo
- **ESC**: Fecha o programa

______________________________________________



## 📌 Observações Finais

- O código pode ser facilmente adaptado para diferentes resoluções alterando as constantes de largura/altura e a matriz de projeção.
- Para adicionar novos mapas, basta criar um novo arquivo de mapa seguindo o formato esperado.
- Para alterar sprites ou tilesets, substitua as imagens nas pastas de assets.
- O jogo foi desenvolvido com uma estrutura modular, o que permite sua expansão com facilidade:
  - É possível adicionar novos tipos de inimigos, obstáculos e itens colecionávei.
  - Novas animações e eventos podem ser incluídos com pequenas alterações no sistema de sprites. Por exemplo, animação para Mochi morrendo ou animação para quando permanece muito tempo parado.
  - Pode-se implementar fases com objetivos diferentes, sistemas de pontuação, ou até mesmo menus e transições entre fases.
  - Também é possível adicionar som ambiente, novos personagens, efeitos sonoros... 


### Bom jogo! 👾
