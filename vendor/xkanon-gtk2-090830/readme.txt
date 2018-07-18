http://gametricks.blog40.fc2.com/blog-entry-103.html

以前からxkanonに執心している私は何度もビルド&インストールに挑戦してきた。

オリジナルソースをビルドしようとする→gtkmmの古いバージョンに依存してて死ぬ
古いgtkmmがあるDebianを試す→他のところでビルドが死ぬ

とつらい思いをしていたのですが、pkgsrcにポートしている人がいまして、それをビルドして満足していました。
https://github.com/tsutsui/pkgsrc-wip-xkanon

で、久しぶりに使おうと思ったらArchでpkgsrcのnbpatchのビルドが止まりつかえない…

そこでなんとなく情報を探していたところ、xkanon-gtkなるものを発見。

消えた作者の日記のはてブなどから察するに、どうやらユーザーからパッチをいただき適用したらしい。

でこいつをビルドしてみたところ、warningが大量に出るもののビルドは完走し、ゲームも起動したのでPKGBUILDを書いてみた

既知の問題としては
?音が出ない。
　--enable-alsaしてみたら途中でフリーズしたりする
?HiDPI環境だとフォントサイズがおかしい
　どうでもいい。--fontsizeでごまかす
?Makefileに存在しないルールを呼び出すルールがある
　謎。しかもそのルールは存在しない.poをmsgfmtしようとするので、
　xkanon-071209.tar.gzからファイルを引っ張って来る羽目に。
?UIが英語になる
　gettextとか.mo周りだと思われる。
を確認した。

とりあえず起動するので直せる人いないかなーと思い書いた。



---

https://gist.github.com/trickart/a744b6e038812fef53fe

    "http://www.creator.club.ne.jp/~jagarl/xkanon-gtk2-090830.tar.gz"
    "http://www.creator.club.ne.jp/~jagarl/xkanon-071209.tar.gz"
    
    