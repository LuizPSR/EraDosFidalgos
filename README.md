# Era dos Fidalgos

# Sobre FLECS

## Monitor WEB

O flecs permite monitorar o estado do sistema na [interface web](https://www.flecs.dev/explorer/).
Basta estar com o jogo rodando, e a interface se conecta a ele no endereço localhost. Assim,
a interface também não é visível fora da máquina local.

## Sistemas

### Registro

Dentro de um sistema, não podemos registrar novos sistemas, porque o
flecs::World fica imutável. É preciso registrá-los antes, ou no fim de
um frame.

O código atualmente usa o loop do flecs, então não
rodamos nada entre frames, apenas os sistemas.
Se for preciso mudar, podemos. Mas por enquanto, registramos os sistemas
dentro de Game::RegisterSystems().

Outra coisa que precisa ser registrada anteriormente são marcadores de
componentes, como Singleton e Sparse.

### Fases
Aqui está uma descrição das diferentes [fases de execução](https://www.flecs.dev/flecs/md_docs_2DesignWithFlecs.html#selecting-a-phase).
dos sistemas.

### Cenas

Para cada cena, nós registramos os sistemas no início e marcamos
como desativados, ao trocar de cena nós ativamos ou desativamos os sistemas
relevantes. A mesma coisa vale para as entidades de cada cena, quando
destruimos uma entidade, todos os componentes atrelados a ela são destruídos
também, então isso facilita na limpeza de cena. Entre entidades, podemos
fazer com que sejam [filhas](https://www.flecs.dev/flecs/md_docs_2Relationships.html#the-childof-relationship)
de uma entidade da cena, e assim quando a cena for destruída as outras
entidades serão também. Também é possível usar scope() para que todas as
entidades definidas dentro do escopo sejam filhas de uma cena.