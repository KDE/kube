/*
 *   Copyright 2017 Christian Mollekopf <mollekopf@kolabsystems.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.7
import QtTest 1.0
import org.kube.framework 1.0 as Kube
import "../mailpartview" as MPV


TestCase {
    id: testCase
    width: 400
    height: 400
    name: "MailViewer"
    when: windowShown
    visible: true

    Kube.File {
        id: file
        path: "/src/kube/framework/qml/tests/kraken.eml"
    }

    Component {
        id: viewerComponent
        Kube.MailViewer {
            anchors.fill: parent
            visible: true
            unread: false
            trash: false
            draft: false
            sent: false
            busy: false
            current: true
            autoLoadImages: true
        }
    }

    function test_1htmlMail() {
        var viewer = createTemporaryObject(viewerComponent, testCase, {message: file.data})

        var htmlButton = findChild(viewer, "htmlButton");
        verify(htmlButton)
        tryVerify(function(){ return htmlButton.visible })
        htmlButton.clicked()

        var htmlView = findChild(viewer, "htmlView");
        verify(htmlView)
        var htmlPart = findChild(viewer, "htmlPart");
        verify(htmlPart)

        tryVerify(function(){ return htmlView.loadProgress == 100})
        tryVerify(function(){ return htmlPart.loaded})

        // What it used to return...
        // tryVerify(function(){ return (1300 > htmlPart.contentWidth && htmlPart.contentWidth > 1200)})
        // tryVerify(function(){ return (2700 > htmlPart.contentHeight && htmlPart.contentHeight > 2600)})
        tryVerify(function(){ return (400 > htmlPart.contentWidth && htmlPart.contentWidth > 300)})
        // Because of the 4000 limit that we have implemented
        tryVerify(function(){ return (4100 > htmlPart.contentHeight && htmlPart.contentHeight > 3900)})
    }

    Component {
        id: htmlComponent
        MPV.HtmlPart {
            anchors.fill: parent
        }
    }

    Component {
        id: textComponent
        MPV.TextPart {
            anchors.fill: parent
        }
    }

    function test_2htmlMail() {
        var data =
"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">
<html><head><title></title><style>
body {
  overflow:hidden;
  font-family: \"Noto Sans\" ! important;
  color: #31363b ! important;
  background-color: #fcfcfc ! important
}
blockquote {
  border-left: 2px solid #bdc3c7 ! important;
}
</style></head>
<body>
<table><tr><td style=\"\">john added a comment.
</td></tr></table><br /><div><div><blockquote style=\"border-left: 3px solid #a7b5bf; color: #464c5c; font-style: italic; margin: 4px 0 12px 0; padding: 4px 12px; background-color: #f8f9fc;\"><blockquote style=\"border-left: 3px solid #a7b5bf; color: #464c5c; font-style: italic; margin: 4px 0 12px 0; padding: 4px 12px; background-color: #f8f9fc;\"><p>This is some text that is quoted.</p></blockquote></blockquote>

<p>I&#039;m afraid due to technical limitations of how connections from us are done that&#039;s not trivial.</p>

<p>I would still suggest scheduling a meeting</p></div></div><br /><div><strong>TASK DETAIL</strong><div><a href=\"https://test.com/T336176\">https://test.com/T336176</a></div></div><br /><div><strong>EMAIL PREFERENCES</strong><div><a href=\"https://test.com/settings/panel/emailpreferences/\">https://test.com/settings/panel/emailpreferences/</a></div></div><br /><div><strong>To: </strong>mollekopf, john<br /><strong>Cc: </strong>john, test, Support<br /></div></body></html>"

        var htmlPart = createTemporaryObject(htmlComponent, testCase, {content: data})
        var htmlView = findChild(htmlPart, "htmlView");
        verify(htmlView)

        tryVerify(function(){ return htmlView.loadProgress == 100})

        tryVerify(function(){ return (450 > htmlPart.contentWidth && htmlPart.contentWidth > 350)})
        tryVerify(function(){ return (500 > htmlPart.contentHeight && htmlPart.contentHeight > 400)})

        var textPart = createTemporaryObject(textComponent, testCase, {content: data})
        var textView = findChild(textPart, "textView");
        verify(textView)
        tryVerify(function(){ return (500 > textView.width && textView.width > 300)})
        tryVerify(function(){ return (500 > textView.height && textView.height > 400)})
    }


    function test_3htmlMail() {
        var data =
"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"
        \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">
<html xmlns=\"http://www.w3.org/1999/xhtml\">
<head>
    <style type=\"text/css\">
        body {
            width: 100% !important;
            -webkit-text-size-adjust: 100%;
            -ms-text-size-adjust: 100%;
            margin: 0;
            padding: 0;
            line-height: 100% !important;
        }

        @media screen and (max-width: 600px) {
            table[class=\"container-table\"] {
                width: 100% !important;
            }

            .h21 {
                height: 21px !important;
            }

            .w25 {
                width: 25px !important;
            }

            .f13 {
                font-size: 13px !important;
            }

            .f19 {
                font-size: 19px !important;
            }

            .f28 {
                font-size: 28px !important;
            }

            .l20 {
                line-height: 20px !important;
            }

            .l21 {
                line-height: 21px !important;
            }

            .l31 {
                line-height: 31px !important;
            }

            .pr0 {
                padding-right: 0px !important;
            }

            .pl0 {
                padding-left: 0px !important;
            }

            .pr10 {
                padding-right: 10px !important;
            }

            .pl10 {
                padding-left: 10px !important;
            }

            .pt25 {
                padding-top: 25px !important;
            }

            .pb25 {
                padding-bottom: 25px !important;
            }

            .hide {
                display: none !important;
            }

            .h15 {
                height: 15px !important;
            }

            .full {
                width: 100% !important;
            }

            .mcenter {
                text-align: center !important;
                margin: 0 auto !important;
            }
        }
    </style>
</head>
<body style=\"\">
<table style=\"padding: 40px 30px 20px 30px;\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"600px\" align=\"center\" class=\"container-table\">

    <tr style=\"\">
        <td style=\"\" class=\"header\">
            <table style=\"\">
                <tr style=\"\">
                    <td style=\"padding-top: 0px; padding-right: 40px; padding-bottom: 15px; padding-left: 40px;\" class=\"pr0 pl0\">
                        <a href=\"https://www.kraken.com/?kec=9WmzuIaQF-wV1c\"><img style=\"display: block; border: none;\" src=\"https://img.kraken.com/e/kraken-site-2019.png\" alt=\"Kraken\"></a>
                    </td>
                </tr>
            </table>
        </td>
    </tr>
    <tr style=\"\">
        <td style=\"\" class=\"body\">
            <table style=\"\">
                <tr style=\"\">
                    <td style=\"\">
                        <table style=\"padding-top: 0px; padding-right: 40px; padding-bottom: 15px; padding-left: 40px; \"
                               class=\"pr0 pl0\">
                            <tr style=\"\">
                                <td style=\"\" class=\"f19\">
                                    <p style=\"margin-top: 1em; padding-bottom: 10px;color: #484848; font-family:Helvetica, Arial, sans-serif; font-size: 21px; line-height: 30px; font-weight: 200;\">
                                        Greetings,
                                    </p>

                                    <p style=\"padding-bottom: 10px;color: #484848; font-family:Helvetica, Arial, sans-serif; font-size: 21px; line-height: 30px; font-weight: 200;\">
                                        You know Kraken. We have a genuine passion for crypto and we=E2=80=99re not afraid to speak our mind or have a unique point of view -- even if it gets us into trouble sometimes ;) We don=E2=80=99t blindly conform, even when it=E2=80=99s popular or safe to do so. Like crypto, we=E2=80=99re not afraid to be different.=20
                                    </p>

                                    <p style=\"padding-bottom: 10px;color: #484848; font-family:Helvetica, Arial, sans-serif; font-size: 21px; line-height: 30px; font-weight: 200;\">
                                        Our branding should strongly reflect our values and personality.
                                    </p>

                                    <p style=\"padding-bottom: 10px;color: #484848; font-family:Helvetica, Arial, sans-serif; font-size: 21px; line-height: 30px; font-weight: 200;\">
                                        We reveal today (to our more than 4,000,000 clients across nearly 200 countries) new branding, including a website redesign.=20
                                    </p>

                                    <p style=\"padding-bottom: 10px;color: #484848; font-family:Helvetica, Arial, sans-serif; font-size: 21px; line-height: 30px; font-weight: 400;\">
                                        Your Guide to Experiencing the New Look of Kraken
                                    </p>

                                    <ul style=\"padding-bottom: 10px;color: #484848; font-family:Helvetica, Arial, sans-serif; font-size: 21px; line-height: 30px; font-weight: 200;\">
                                            <li><span style=\"font-weight:400;\"\">We are Kraken.</span> Don=E2=80=99t worry -- we=E2=80=99re not changing our name. Kraken is two syllables, fun to say, memorable, resonates with you emotionally, conjures strong mental imagery, doesn=E2=80=99t restrict our future offerings (e.g., no =E2=80=9Cbit=E2=80=9D, =E2=80=9Ccoin=E2=80=9D, =E2=80=9Cblock=E2=80=9D, =E2=80=9Cchain=E2=80=9D, =E2=80=9Ccrypto=E2=80=9D, etc.), contains an inherent sense of suspense and excitement, and plays on trading themes like liquidity, depth, dark pools, etc.</li>
                                            <li><span style=\"font-weight:400;\">Kraken stands out in a sea of boring blue.</span> A lot of companies, especially tech and financial services, like to use the color blue. It=E2=80=99s a safe and popular color, as well as associated with trust, honesty and dependability. The problem is that too many companies and brands use this color. As a result, blue then may as well be a boring beige. Bye-bye blue! Today we welcome a bolder color that is more unique, better represents our personality and is guaranteed to grab your attention. This new main color evokes feelings of richness and prestige, which is fitting considering Kraken has handled over $140,000,000,000 in trades and is one of the oldest and most respected crypto exchanges. </li>
                                            <li><span style=\"font-weight:400;\">A colorful industry deserves a colorful palette.</span> Something we really like about the crypto industry is its people. There=E2=80=99s something about this space that attracts the outspoken, devoted, opinionated, and eccentric. Our clients and team members are no different -- they are the most bright, colorful and interesting minds. In addition to the bold crypto community, the industry itself is constantly evolving -- it is alive, vibrant, fresh, and impactful. We have chosen to drape Kraken in a deliciously fitting and bold color scheme. </li>
                                            <li><span style=\"font-weight:400;\">Crypto is a rabbit hole, and we are all Alice.</span> Since learning about and experiencing crypto you may feel that you=E2=80=99ve been living a much more exhilarating, suspenseful, dramatic, and purposeful life. I guess that=E2=80=99s what happens when you join a global revolution. Crypto is a grand adventure that keeps you on the edge of your seat. It beckons the creative, rebellious and determined dreamers to rise and take action. This provocative spirit of crypto is brought to life in our new, adventure-themed illustrations. As you move around our site, allow your eyes to linger and get lost in the beautiful and quirky illustrations. </li>
                                            <li><span style=\"font-weight:400;\">Are you ready for us, consumer market?</span> Since 2013, we=E2=80=99ve been one of the most popular places to buy, sell, or trade crypto. A popular segment for us has always been pro traders who wield our advanced trading features and charts to turn profits. Our plan has been and will be to always keep our traders top of mind while expanding our appeal to all other segments and markets. In particular, we have known that our brand, personality and company mission have great potential in resonating with the consumer market. We haven=E2=80=99t been shy about <a href=\"https://www.kraken.com/why-kraken\">stating our purpose.</a> Kraken is here to pave the way for mass adoption of crypto and digital assets so that everyone can have financial freedom and inclusion. You will see our tone of voice, look and feel, products, UX, and interactions become more approachable, streamlined, easy-to-consume, and more cool and fun. Crypto is really freakin=E2=80=99 cool after all. It=E2=80=99s about time that the world knows this, too :)</li>
                                            <li><span style=\"font-weight:400;\">Kraken will continue to evolve.</span> Expect more improvements made and illustrations added to the website, as well as advancements in trading interfaces and products, like [insert trade.kraken.com link]. <a href=\"https://blog.kraken.com/post/2035/\">Go to our blog</a> to learn more about our rebranding efforts.</li>
                                    </ul>

                                    <p style=\"padding-bottom: 10px;color: #484848; font-family:Helvetica, Arial, sans-serif; font-size: 21px; line-height: 30px; font-weight: 200;\">
                                            Now you=E2=80=99re prepared for your crypto journey. Go to <a href=\"https://www.kraken.com/\">Kraken</a> and get pulled into the intoxicating adventure that awaits you.=20
                                    </p>

                                    <p style=\"padding-bottom: 10px; color: #484848; font-family:Helvetica, Arial, sans-serif; font-size: 21px; line-height: 30px; font-weight: 200;\">
                                        Thank you for choosing Kraken, the trusted and secure digital assets exchange.
                                    </p>

                                    <p style=\"padding-bottom: 10px;color: #484848; font-family:Helvetica, Arial, sans-serif; font-size: 21px; line-height: 30px; font-weight: 200;\">
                                        The Kraken Team
                                    </p>
                                    <!-- android spacer -->
                                    <table style=\"height: 4px;\" class=\"h15\">
                                        <tr style=\"\">
                                            <td height=\"1\" class=\"hide\" style=\"min-width:640px; font-size:0px;line-height:0px;\">
                                                <img height=\"1\" src=\"https://www.kraken.com/img/kraken_dot.png?kec=9WmzuIaQF-wV1c\" style=\"min-width: 640px; text-decoration: none; border: none; -ms-interpolation-mode: bicubic;\" alt=\"\">
                                            </td>
                                        </tr>
                                    </table>
                                    <!-- Close spacer Gmail Android -->
                                    <table style=\"height: 14px;\" class=\"h15\">
                                        <tr style=\"\">
                                            <td height=\"1\" class=\"hide\" style=\"min-width:640px; font-size:0px;line-height:0px;\">
                                                <img height=\"1\" src=\"https://www.kraken.com/img/kraken_dot.png\" style=\"min-width: 640px; text-decoration: none; border: none; -ms-interpolation-mode: bicubic;\" alt=\"\">
                                            </td>
                                        </tr>
                                    </table>
                                </td>
                            </tr>
                        </table>
                    </td>
                </tr>
            </table>
        </td>
    </tr>
    <tr style=\"\">
        <td>
            <hr style=\"padding:0; border:none; margin-bottom:25px; background-color:#dbdbdb; height:1px;\">
        </td>
    </tr>
    <tr>
        <td style=\"\" class=\"footer\">
            <table style=\"\">
                <tr style=\"\">
                    <td style=\"\" class=\"smedia\">
                        <table style=\"\">
                            <tr style=\"\">
                                <td style=\"font-family:Helvetica, Arial, sans-serif; font-size: 16px; line-height: 17px; color: #9B9B9B;\" class=\"f19 l21\">
                                    spread the word
                                </td>
                                <td style=\"padding: 0 5px;\" class=\"pr10 pl10\">
                                    <a style=\"\" href=\"https://twitter.com/krakenfx\"  target=\"_blank\">
                                        <img src=\"https://img.kraken.com/e/twitter.png\" width=\"14\" height=\"13\" style=\"display: block; border: none;\" alt=\"Twitter\" class=\"h21 w25\">
                                    </a>
                                </td>
                                <td style=\"padding: 0 5px;\" class=\"pl10\">
                                    <a style=\"\" href=\"https://www.facebook.com/KrakenFX/\"  target=\"_blank\">
                                        <img src=\"https://img.kraken.com/e/facebook.png\" width=\"14\" height=\"13\" style=\"display: block; border: none;\" alt=\"Facebook\" class=\"h21 w25\">
                                    </a>
                                </td>
                            </tr>
                        </table>
                    </td>
                </tr>
                <tr style=\"\">
                    <td style=\"padding: 20px 0; font-family:Helvetica, Arial, sans-serif; font-size: 11px; line-height: 17px; font-weight: 200;\" class=\"f13 l20\">
                        This communication is a commercial message and its contents are intended for the recipient only and
                        may contain confidential, non-public and/or privileged information. If you have received this
                        communication in error, do not read, duplicate or distribute. Kraken does not make recommendations
                        on the suitability of a particular asset class, strategy, or course of action. Any investment
                        decision you make is solely your responsibility. Please consider your individual position and
                        financial goals before making an independent investment decision. <br>
                        Distributed by: Kraken.com, 237 Kearny St #102, San Francisco, CA 94108.
                    </td>
                </tr>
                <tr style=\"\">
                    <td style=\"font-family:Helvetica, Arial, sans-serif; font-size: 13px; line-height: 17px; color: #9B9B9B;\">
                        This email contains important updates from Kraken. <a target=\"_blank\" href=\"https://www.kraken.com/unsubscribe?kec=9WmzuIaQF-wV1c\" style=\"color: #506D88; text-decoration: underline;\">Unsubscribe</a>
                    </td>
                </tr>
            </table>
        </td>
    </tr>
</table>
</body>
</html>"

        var htmlPart = createTemporaryObject(htmlComponent, testCase, {content: data, autoLoadImages: true})
        var htmlView = findChild(htmlPart, "htmlView");
        verify(htmlView)
        tryVerify(function(){ return htmlView.loadProgress == 100})
        tryVerify(function(){ return htmlPart.loaded})

        // tryVerify(function(){ return (1300 > htmlPart.contentWidth && htmlPart.contentWidth > 1200)})
        // tryVerify(function(){ return (2800 > htmlPart.contentHeight && htmlPart.contentHeight > 2700)})
        tryVerify(function(){ return (500 > htmlPart.contentWidth && htmlPart.contentWidth > 400)})
        tryVerify(function(){ return (4100 > htmlPart.contentHeight && htmlPart.contentHeight > 3900)})
    }


}
